// primes.c

#include "../kernel/types.h"
#include "../user/user.h"

#define READ 0
#define WRITE 1
#define MAX 36
#define stderr 2

void child(int *p) {
    // 关闭上一个进程的管道的写端口
    close(p[WRITE]);

    int i;
    int read_r = read(p[READ], &i, sizeof(int));
    if (read_r == 0) {
        // 没有数被传到这个进程来 该结束了
        close(p[READ]);
        exit(0);
    } else if (read_r > 0) {
        // 每次，或者说每个子进程只打印当前那个最小的数，也即那个素数
        // 第一个数
        printf("prime %d\n", i);
    } else {
        // -1
        printf("get wrong!\n");
        exit(1);
    }

    // 先创建管道，关闭读端口
    int temp_p[2];
    pipe(temp_p);

    if (fork() == 0) {
        child(temp_p);
    } else {
        // 首先关闭与下一个进程交流的管道的读端口
        close(temp_p[READ]);

        int j;

        while ((read_r = read(p[READ], &j, sizeof(int))) > 0) {
            // 如果不会被整除，就把它塞到新管道中。让下一个进程处理
            if (j % i != 0)
            {
                write(temp_p[WRITE], &j, sizeof(int));
            }

        }

        if (read_r < 0) {
            fprintf(2, "cat: read error\n");
            exit(1);
        }
        close(p[READ]);
        close(temp_p[WRITE]);

        // 等待，等到了子进程返回，自己也结束了
        wait(0);
        exit(0);
    }
}

int main() {
    int p[2];
    pipe(p);

    int pid = fork();

    if (pid == 0) {
        child(p);

    } else {
        close(p[READ]);

        for (int i = 2; i < MAX; i++) {
            write(p[WRITE], &i, sizeof(int));
        }

        close(p[WRITE]);
        wait(0);
    }

    exit(0);
}