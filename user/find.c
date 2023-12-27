// find.c
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../user/user.h"
#include "../kernel/fs.h"

#define READ 0
#define WRITE 1
#define MAX 36
#define stderr 2

/*
 char *formatted_name = fmtname("/home/user/example_file.txt");
 假设 DIRSIZ 为 20
 如果文件名长度超过了 DIRSIZ，直接返回原始文件名指针
 假设文件名长度不超过 DIRSIZ，则格式化后的文件名可能是这样的：
 formatted_name = "example_file.txt     "
 */

const char * trimTrailingSpaces(char *str) {
    int len = 0;
    int i;

    // 计算字符串长度
    while (str[len] != '\0') {
        len++;
    }

    // 从字符串末尾开始向前遍历，找到最后一个非空格字符的位置
    for (i = len - 1; i >= 0; i--) {
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r') {
            break;
        }
    }

    // 在最后一个非空格字符后面添加字符串结束符
    str[i + 1] = '\0';

    return str;
}

char *
fmtname(char *path) {
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    // 从路径的末尾开始向前遍历，直到找到最后一个斜杠前的字符或者到达路径的起始位置为止。
    // 这个循环将 p 指针移动到最后一个斜杠后的第一个字符的位置。如果路径名以斜杠结尾，则 p 指向字符串结束符 \0。
    for (p = path + strlen(path); p >= path && *p != '/'; p--);
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

void find(char *path, char *targetFileName) {
    char buf[512], *p;
    int fd;
    struct dirent de; //用于表示目录项的结构体
    struct stat st; // 文件描述符 fd 对应文件的状态信息


    // 尝试打开指定路径的文件描述符
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }
    // 是一个系统调用，用于获取指定文件描述符 fd 对应文件的状态信息
    // 并将这些信息存储在 struct stat st 结构体中。
    if (fstat(fd, &st) < 0) {
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }
    // 根据文件类型进行不同的操作
    switch (st.type) {
        case T_FILE:// 如果是文件
            if (strcmp(targetFileName, trimTrailingSpaces(fmtname(path))) == 0) {
                printf("%s\n", fmtname(path));
            }

            break;

        case T_DIR:// 如果是目录

            // 判断路径长度是否超过缓冲区大小
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("ls: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            // 目录 a -> a/
            // 读取目录项
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                // 索引节点号为0的目录项通常表示未使用的目录项或者空白的目录项
                if (de.inum == 0)
                    continue;
                // 使用了 memmove 函数来将目录项中的文件名（de.name）复制到缓冲区 p 中。
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                // 获取每个文件的详细信息
                if (stat(buf, &st) < 0) {
                    printf("ls: cannot stat %s\n", buf);
                    continue;
                }

                if (strcmp(targetFileName, trimTrailingSpaces(fmtname(buf))) == 0) {
                    printf("%s\n", buf);
                }

                if (st.type == T_DIR && strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
                    find(buf, targetFileName);
                }
            }
            break;
    }
    close(fd);// 关闭文件描述符
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: find [menu] [path]\n");
        exit(0);
    }

    find(argv[1], argv[2]);

    exit(0);
}

/*
mkdir IaTSruRZ
echo > IaTSruRZ/o1gZLJJ7
find . o1gZLJJ7
 * */