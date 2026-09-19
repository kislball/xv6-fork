#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  char a = 'A', b = 'B';
  if (argc >= 2)
    a = argv[1][0];
  if (argc == 3) {
    b = argv[2][0];
  }

  int result = intsum(a, b);

  exit(result);
}
