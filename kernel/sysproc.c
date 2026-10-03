#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

extern struct proc proc[NPROC];
extern struct spinlock wait_lock;

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.gb
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
dump_proc_into_user(pagetable_t pgt, uint64 sz, uint64 us_addr, int lim)
{
  int proccount = 0;

  acquire(&wait_lock);
  for (int i = 0; i < NPROC; i++) {
    struct procinfo cur_info;

    acquire(&proc[i].lock);
    if (proc[i].state == UNUSED || proc[i].state == USED) {
      release(&proc[i].lock);
      continue;
    }

    if (proccount < lim) {
      cur_info.pid = proc[i].pid;
      cur_info.ppid = proc[i].parent == 0 ? 0 : proc[i].parent->pid;
      safestrcpy(cur_info.name, proc[i].name, sizeof(cur_info.name));
      cur_info.state = proc[i].state;
    }
    release(&proc[i].lock);

    if (proccount < lim && copyout(pgt, sz, us_addr + proccount * sizeof(cur_info),
                                   (char *)&cur_info, sizeof(cur_info)) < 0) {
      release(&wait_lock);
      return -1;
    }
    proccount++;
  }
  release(&wait_lock);

  return proccount;
}

uint64
sys_listproc(void)
{
  uint64 address_raw;
  int limit;

  argaddr(0, &address_raw);
  argint(1, &limit);

  if (address_raw == 0 || limit <= 0) {
    return -1;
  }

  struct proc *cur = myproc();
  pagetable_t pgt = cur->pagetable;
  uint64 page_size = cur->sz;

  return dump_proc_into_user(pgt, page_size, address_raw, limit);
}
