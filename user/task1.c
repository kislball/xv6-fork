#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{

  if (argc < 2) {
    printf("not enougth arguments\n");
    exit(1);
  }

  const char letter = argv[1][0];

  int child_pid = fork();

  if (child_pid == -1) {
    exit(1);
  }
  // child process
  if (child_pid == 0) {
    pause(150); // ~ 15 секунд
    exit(1);
  }
  // parent process
  else {
    int self_pid = getpid();
    printf("parent_pid: %d\n", self_pid);
    printf("child_pid: %d\n", child_pid);

    if (letter == 'a') {
      int status;
      int ended_child_pid = wait(&status);
      printf("variant a, child_pid: %d, exit_code: %d\n", ended_child_pid,
             status);
    } else if (letter == 'b') {
      int status;
      int kill_code = kill(child_pid);
      if (kill_code != -1) {
        int ended_child_pid = wait(&status);
        printf("variant b, child_pid: %d, exit_code: %d\n", ended_child_pid,
               status);
      } else {
        printf("could not kill child proccess\n");
        exit(1);
      }
    } else {
      printf("no letters recognised\n");
      exit(1);
    }
  }
  exit(0);
}
