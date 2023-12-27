// xargs.c
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../user/user.h"
#include "../kernel/fs.h"
#include "../kernel/param.h"


#define READ 0
#define WRITE 1
#define MAX 36
#define stderr 2

char *
ugets(char *buf, int max) {



    int i, cc;
    char c;

    for (i = 0; i + 1 < max;) {
        cc = read(READ, &c, 1);
        if (cc < 1) {
            break;
        }
        if (c == '\n' || c == '\r' || c == ' ') {
            break;
        }
        buf[i++] = c;
//        printf("buf: %c, c: %c\n", buf, c);
    }
    buf[i] = '\0';

    return buf;
}

// sh.c 的 getcmd 改造而来
/*
 * getcmd，用于从标准输入中获取用户输入的命令，并将其存储在提供的缓冲区 buf 中
 * */
int ugetcmd(char *buf, int nbuf) {
    // 使用 memset 函数将 buf 缓冲区的前 nbuf 字节清零，以确保缓冲区为空。
    memset(buf, 0, nbuf);
    ugets(buf, nbuf);
    if (buf[0] == 0) // EOF
        return -1;
    return 0;
}

// 向字符指针数组后面添加一个指针
void append1(int *argc, char *argv[], char *token)
{
    argv[*argc] = token;
    (*argc)++;
    argv[*argc] = 0;
}

int main(int argc, char *argv[]) {
    /*
    argv[0] = "xargs"
    argv[1] = command
    argv[2] = para
    ...
    argv[END] = 0
     ->
    */

    // echo line
    char *argv_alter[MAXARG];
    int argc_alter = argc - 1;

    for (int i = 0; i < argc_alter; i++) {
        argv_alter[i] = argv[i + 1];
    }

    char buf[100];
    // 存储输入的参数
    char token[MAXARG][100];

    int token_len = 0;
    while (ugetcmd(buf, sizeof(buf)) >= 0) {
        strcpy(token[token_len++], buf);
//        printf("buf: %s", buf);
    }

    for (int i = 0; i < token_len; i++) {
        if (fork() == 0) {
            append1(&argc_alter, argv_alter, token[i]);

            exec(argv_alter[0], argv_alter);

            // 失败了就退出了
            printf("exec failed!\n");
            exit(1);
        } else {
            wait(0);
        }
    }

    exit(0);
}

/*
echo hello | xargs echo line
echo "1\n2" | xargs echo line
 * */