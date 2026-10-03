#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/riscv.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  static char *states[] = {
    // clang-format off
    [UNUSED]    = "unused",
    [USED]      = "used",
    [SLEEPING]  = "sleep ",
    [RUNNABLE]  = "runble",
    [RUNNING]   = "run   ",
    [ZOMBIE]    = "zombie"
    // clang-format on
  };
  int lim = NPROC;
  struct procinfo plist[NPROC];
  int proccount = listproc(plist, lim);
  char *state;

  if (proccount < 0) {
    fprintf(2, "ps: listproc failed\n");
    exit(1);
  }

  printf("PID STATE NAME PPID\n");
  for (int pc = 0; pc < proccount; pc++) {
    struct procinfo *p = &plist[pc];
    state = "???";

    if (p->state >= 0 && p->state < sizeof(states) / sizeof(states[0]) && states[p->state])
      state = states[p->state];
    printf("%d %s %s %d\n", p->pid, state, p->name, p->ppid);
  }

  exit(0);
}
