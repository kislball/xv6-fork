#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define READ_BUF_SIZE 4096

// Tries to write data to FD completely.
// Writes left bytes on write() interruptions.
// On success returns 0, otherwise -1.
int writeAll(int fileDescriptor, char* startPos, ssize_t totalBytes) {
    for (ssize_t sentBytes = 0; sentBytes < totalBytes;) {
        ssize_t writtenBytes =
            write(fileDescriptor, startPos + sentBytes, totalBytes - sentBytes);

        if (writtenBytes == -1) {
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        sentBytes += writtenBytes;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    int p[2];

    if (pipe(p) < 0) {
        fprintf(stderr, "can't create pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "can't create fork");
        exit(1);
    }

    if (pid > 0) {
        close(p[0]);

        for (int i = 0; i < argc; i++) {
            if (writeAll(p[1], argv[i], strlen(argv[i])) == -1) {
                close(p[1]);
                fprintf(stderr, "can't write data to pipe");
                exit(2);
            }

            if (writeAll(p[1], "\n", 1) == -1) {
                close(p[1]);
                fprintf(stderr, "can't write data to pipe");
                exit(2);
            }
        }

        close(p[1]);
    }

    if (pid == 0) {
        close(p[1]);
        char buf[READ_BUF_SIZE];

        for (;;) {
            ssize_t readBytes = read(p[0], buf, READ_BUF_SIZE - 1);
            if (readBytes == -1) {
                if (errno == EINTR) {
                    continue;
                }

                close(p[0]);
                fprintf(stderr, "can't read data from pipe");
                exit(3);
            }

            if (readBytes == 0) {
                break;
            }

            buf[readBytes] = '\0';
            printf("%s", buf);
        }

        close(p[0]);
    }
}
