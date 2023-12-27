
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../user/user.h"

#define stdout 1
#define stderr 2
#define READ 0
#define WRITE 1

int
main() {
    // fdp2c : file descriptor, parent to child
    int fdp2c[2], fdc2p[2], pid;
    char p2c[64] = {0}, c2p[64] = {0}; // 初始化缓冲区

    if (pipe(fdp2c) < 0 || pipe(fdc2p) < 0) {
        fprintf(stderr, "Pipe creation failed.\n");
        exit(1);
    }

    pid = fork();
    // child
    if (pid == 0) {
        close(fdp2c[WRITE]);
        close(fdc2p[READ]); // 关闭子进程的读取端

        // read -- 父进程必须是 write
        read(fdp2c[READ], p2c, sizeof(p2c));// 如果没消息会阻塞，等待父进程传来消息
        printf("%d: received %s\n", getpid(), p2c);

        write(fdc2p[WRITE], "pong", sizeof("pong"));


        close(fdp2c[READ]);
        close(fdc2p[WRITE]);  // 写入完成后关闭写入端
        exit(0);
    } else {
        // parent
        close(fdc2p[WRITE]);  // 关闭父进程的写入端

        close(fdp2c[READ]);

        write(fdp2c[WRITE], "ping", sizeof("ping"));

        read(fdc2p[READ], c2p, sizeof(c2p));

        printf("%d: received %s\n", getpid(), c2p);

        close(fdc2p[READ]);

        close(fdp2c[WRITE]);
    }
    exit(0);
}