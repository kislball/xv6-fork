enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct procinfo {
  int pid;
  char name[16];
  int ppid;
  enum procstate state;
};
