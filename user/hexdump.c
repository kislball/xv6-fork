#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  static const char digits[] = "0123456789ABCDEF";
  if (argc < 3)
    return 1;
  int count = atoi(argv[1]);
  int fd = open(argv[2], O_RDONLY);
  uchar *buf = (uchar *)sbrk(count);

  int n = read(fd, buf, count);
  close(fd);
  if (n < 0) {
    fprintf(2, "Read error\n");
    exit(1);
  }

  for (int i = 0; i < n; i++) {
    uchar byte = buf[i];
    char hex[] = {digits[byte >> 4], digits[byte & 0xf],
                  i == n - 1 ? '\n' : ' '};
    write(1, hex, sizeof(hex));
  }
  exit(0);
}
