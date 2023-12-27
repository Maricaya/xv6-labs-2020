#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../user/user.h"
#include "../kernel/fs.h"

// 格式化文件名
// 这个函数的目的是确保文件名达到一定的长度（DIRSIZ）
// 如果文件名本身已经足够长，就直接返回，否则在文件名末尾填充空格，使其达到预定义的长度。
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

void
ls(char *path) {
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
            printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
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
                printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
            }
            break;
    }
    close(fd);// 关闭文件描述符
}

int
main(int argc, char *argv[]) {
    int i;
    if (argc < 2) {
        ls(".");
        exit(0);
    }
    for (i = 1; i < argc; i++)
        ls(argv[i]);
    exit(0);
}
