#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "kernel/defs.h"
#include <stdlib.h>

#include "user/user.h"

int main(int argc, char* argv[]) {
  static char* states[] = {
      // clang-format off
    [UNUSED]    = "unused",
    [USED]      = "used",
    [SLEEPING]  = "sleep ",
    [RUNNABLE]  = "runble",
    [RUNNING]   = "run   ",
    [ZOMBIE]    = "zombie"
      // clang-format on
  };

  int lim = 64;

  struct procinfo *plist = (struct procinfo*)malloc(sizeof(struct procinfo) * lim);
  //ps_listinfo(plis, lim);
  struct procinfo* p;
  char* state;

  fprintf(2, "\n");
  for (int pc = 0; pc < 64; pc++) {
    p = &plist[pc];
    if (p->state == UNUSED) continue;
    if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    fprintf(2, "%d %s %s %d ", p->pid, state, p->name, p->parent->pid);
    fprintf(2, "\n");
  }
}
