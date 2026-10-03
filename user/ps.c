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
  int lim = 2;
  int proccount;
  struct procinfo *plist;
  struct procinfo *p;
  char *state;

  for (;;) {
    plist = malloc(sizeof(struct procinfo) * lim);
    if (plist == 0) {
      fprintf(2, "ps: out of memory\n");
      exit(1);
    }

    proccount = listproc(plist, lim);
    if (proccount < 0) {
      free(plist);
      fprintf(2, "ps: listproc failed\n");
      exit(1);
    }
    if (proccount <= lim)
      break;

    free(plist);
    lim = proccount;
  }

  printf("PID STATE NAME PPID\n");
  for (int pc = 0; pc < proccount; pc++) {
    p = &plist[pc];
    state = "???";

    if (p->state >= 0 && p->state < sizeof(states) / sizeof(states[0]) &&
        states[p->state])
      state = states[p->state];
    printf("%d %s %s %d\n", p->pid, state, p->name, p->ppid);
  }

  free(plist);
  exit(0);
}
