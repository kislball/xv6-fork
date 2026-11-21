#include "defs.h"
#include "file.h"
#include "fs.h"
#include "sleeplock.h"
#include "spinlock.h"
#include "types.h"

enum nulldevice_kind_t {
    NULLDEVICE_NULL,
    NULLDEVICE_ZERO,
    NULLDEVICE_NULLSTAT,

    // add new types here, if needed

    NULLDEVICE_TOTAL
};

struct nullfilehandler {
  int (*read)(int, uint64, int);
  int (*write)(int, uint64, int);
};

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

struct nullfilehandler handlers[NULLDEVICE_TOTAL] = {
    [NULLDEVICE_NULL] = {},
    [NULLDEVICE_ZERO] = {zeroread, zerowrite},
    [NULLDEVICE_NULLSTAT] = {}
};

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
    devsw[NULLDEVICES].write = nullwrite;
    devsw[NULLDEVICES].read = nullread;
}
