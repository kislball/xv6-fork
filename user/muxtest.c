#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int
print_args(int mux, int argc, char *argv[])
{
  int pid = getpid();
  for (int i = 1; i < argc; i++) {
    int len = strlen(argv[i]);
    for (int j = 0; j < len; j++) {
      if (mux != -1 && mutexlock(mux) < 0) {
        fprintf(2, "muxtest: mutexlock failed\n");
        return -1;
      }
      printf("%d: arg %d, char '%c'\n", pid, i, argv[i][j]);
      if (mux != -1 && mutexunlock(mux) < 0) {
        fprintf(2, "muxtest: mutexunlock failed\n");
        return -1;
      }
    }
  }
  return 0;
}

static int
run_phase(int mux, int argc, char *argv[])
{
  int workers = 0;
  int status = 0;
  for (int i = 0; i < 2; i++) {
    int child = fork();
    if (child < 0) {
      fprintf(2, "muxtest: fork failed\n");
      status = 1;
      break;
    }
    if (child == 0) {
      int child_status = print_args(mux, argc, argv) < 0;
      if (mux != -1 && close(mux) < 0) {
        fprintf(2, "muxtest: close failed\n");
        child_status = 1;
      }
      exit(child_status);
    }
    workers++;
  }

  if (mux != -1 && close(mux) < 0) {
    fprintf(2, "muxtest: close failed\n");
    status = 1;
  }

  for (int i = 0; i < workers; i++) {
    int child_status;
    if (wait(&child_status) < 0) {
      fprintf(2, "muxtest: wait failed\n");
      status = 1;
    } else if (child_status != 0) {
      status = 1;
    }
  }
  return status;
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "usage: muxtest arg ...\n");
    return 1;
  }

  printf("Without mutex:\n");
  if (run_phase(-1, argc, argv) != 0)
    return 1;

  int mux = mutex();
  if (mux < 0) {
    fprintf(2, "muxtest: mutex creation failed\n");
    return 1;
  }
  printf("With mutex:\n");
  return run_phase(mux, argc, argv);
}
