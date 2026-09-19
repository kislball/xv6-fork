#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define READ_BUF_SIZE 4096
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
            size_t len = strlen(argv[i]);
            if (write(p[1], argv[i], len) < len) {
                fprintf(stderr, "can't send argument %s completely", argv[i]);
            }
            if (write(p[1], "\n", 1) < 1) {
                fprintf(stderr, "can't send \\n");
            }
        }

        close(p[1]);
    }

    if (pid == 0) {
        close(p[1]);
        char buf[READ_BUF_SIZE];
        ssize_t readBytes;

        do {
            readBytes = read(p[0], buf, READ_BUF_SIZE - 1);
            buf[readBytes] = '\0';
            printf("%s", buf);
        } while (readBytes == READ_BUF_SIZE);

        close(p[0]);
    }
}
