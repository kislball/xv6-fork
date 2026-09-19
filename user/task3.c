#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p[2];
  char *argvv[2];
  argvv[0] = "wc";
  argvv[1] = 0;
  pipe(p);
  if (fork() == 0) {
    close(0);
    dup(p[0]);
    close(p[0]);
    close(p[1]);
    exec("/wc", argvv);
  } else {
    close(p[0]);
    for (int i = 0; i < argc; i++) {
      write(p[1], argv[i], strlen(argv[i]));
      if (i + 1 < argc) {
        write(p[1], " ", 1);
      } else {
        write(p[1], "\n", 1);
      }
    }
    close(p[1]);
    wait(0);
  }
  exit(0);
}
