#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_intsum(void)
{
  int a, b;
  argint(0, &a);
  argint(1, &b);

  struct proc *p = myproc();
  printk("intsum: pid=%d %s, a=%d, b=%d, a+b=%d\n", p->pid, p->name, a, b, a + b);

  return a + b;
}
