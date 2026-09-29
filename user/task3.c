#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/user.h"

#define CHUNK_SIZE 512

int main(int argc, char* argv[]) {
  int p[2];
  char* argvv[2];
  argvv[0] = "wc";
  argvv[1] = 0;
  pipe(p);

  if (pipe(p) < 0) {
    fprintf(2, "Pipe error\n");
    exit(1);
  }

  int f = fork();
  if (f < 0) {
    fprintf(2, "Fork error\n");
    exit(1);
  }

  if (f == 0) {
    close(0);
    dup(p[0]);
    if (dup(p[0]) < 0) {
      fprintf(2, "Dup error\n");
      exit(1);
    }

    close(p[0]);
    close(p[1]);
    exec("/wc", argvv);

    fprintf(2, "Exec error\n");
    exit(1);
  } else if (f > 0) {
    close(p[0]);
    for (int i = 0; i < argc; i++) {
      int total_bytes = strlen(argv[i]);
      int bytes_written = 0;

      while (bytes_written < total_bytes) {
        int bytes_to_write = total_bytes - bytes_written;

        if (bytes_to_write > CHUNK_SIZE) {
          bytes_to_write = CHUNK_SIZE;
        }

        int n = write(p[1], argv[i] + bytes_written, bytes_to_write);
        if (n <= 0) {
          fprintf(2, "Write error\n");
          close(p[1]);
          wait(0);
          exit(1);
        }

        bytes_written += n;
      }
      if (i + 1 < argc) {
        if (write(p[1], " ", 1) <= 0) {
          fprintf(2, "Write error\n");
          break;
        }
      } else {
        if (write(p[1], "\n", 1) <= 0) {
          fprintf(2, "Write error\n");
          break;
        }
      }
    }

    close(p[1]);
    wait(0);
  }

  exit(0);
}
