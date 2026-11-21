#include "types.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"

struct nullfilehandler {
  int (*read)(int, uint64, int);
  int (*write)(int, uint64, int);
};

struct {
  struct spinlock lock;
  uint64 total;
} nullstat;

void
nullstat_init()
{
  nullstat.total = 0;
  initlock(&nullstat.lock, "nullstat");
}

int
nullstat_write(int user_src, uint64 src, int n)
{
  acquire(&nullstat.lock);
  nullstat.total += n;
  release(&nullstat.lock);
  return n;
}

int
nullstat_read(int user_dst, uint64 dst, int n)
{
  if (n != sizeof(uint64))
    return -1;

  acquire(&nullstat.lock);
  int res =
    either_copyout(user_dst, dst, &nullstat.total, sizeof(nullstat.total));
  release(&nullstat.lock);
  return res == -1 ? -1 : sizeof(nullstat.total);
}

int
zeroread(int user_dst, uint64 dst, int n)
{
  char zeros[64] = {0};
  int left = n;
  while (left > 0) {
    int chunk = left < sizeof(zeros) ? left : sizeof(zeros);
    if (either_copyout(user_dst, dst, zeros, chunk) < 0) {
      return -1;
    }
    dst += chunk;
    left -= chunk;
  }
  return n;
}

int
zerowrite(int user_src, uint64 src, int n)
{
  return -1;
}

int
nulldevread(int user_dst, uint64 dst, int n)
{
  return 0;
}

int
nulldevwrite(int user_src, uint64 src, int n)
{
  return n;
}

struct nullfilehandler handlers[NULLDEVICE_TOTAL] = {
  [NULLDEVICE_NULL] = {nulldevread, nulldevwrite},
  [NULLDEVICE_ZERO] = {zeroread, zerowrite},
  [NULLDEVICE_NULLSTAT] = {.read = nullstat_read, .write = nullstat_write}};

int
nullwrite(int user_src, uint64 src, int n, short minor)
{
  return handlers[minor].write(user_src, src, n);
}

int
nullread(int user_dst, uint64 dst, int n, short minor)
{
  return handlers[minor].read(user_dst, dst, n);
}

void
nullinit(void)
{
  nullstat_init();

  devsw[NULLDEVICES].write = nullwrite;
  devsw[NULLDEVICES].read = nullread;
}
