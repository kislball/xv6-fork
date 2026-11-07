#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// just a test to see whether syscall works correctly

int
main(int argc, char *argv[])
{
  pages();
  exit(0);
}
