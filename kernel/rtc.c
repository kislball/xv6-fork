#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "proc.h"
#include "defs.h"
#include "fs.h"
#include "file.h"

#define Reg(reg) ((volatile uint32 *)(RTC_BASE + (reg)))

struct spinlock lock;

int
freadtime(int user_dst, uint64 dst, int n, short minor)
{
    if (n != sizeof(uint64)) return -1;
    uint64 time = readtime();
    return either_copyout(user_dst, dst, &time, sizeof(time)) == 0 ? n : -1;
}

int
fwritetime(int user_dst, uint64 dst, int n, short minor)
{
    return -1;
}

void
inittime(void)
{
    initlock(&lock, "time");

    devsw[RTC_DEVICE].read = freadtime;
    devsw[RTC_DEVICE].write = fwritetime;
}

uint64
readtime(void)
{
    acquire(&lock);
    uint32 time_low = *Reg(0);
    uint32 time_high = *Reg(4);    
    release(&lock);

    uint64 time_low64 = time_low;
    uint64 time_high64 = time_high;

    return time_low64 | (time_high64 << 32);
}