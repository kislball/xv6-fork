#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    printf("Incorrect input\n");
    exit(1);
  }
  int a = atoi(argv[1]);
  int b = atoi(argv[2]);

  int result = intsum(a, b);
  printf("%d+%d=%d\n", a, b, result);

  exit(0);
}
