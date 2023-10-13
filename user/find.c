#include "kernel/types.h"

#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"
/**
 * @brief 从指定路径查找文件
 * @param path 路径
 * @param filename 文件名
*/
void find(char* path, const char *filename) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // open函数返回路径的文件描述符
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    // fstat函数将文件描述符映射到stat结构体上
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot fstat %s\n", path);
        close(fd);
        return;
    }
    // 只能是目录
    if (st.type != T_DIR) {
        fprintf(2, "usage: find <DIRECTORY> <filename>\n");
        return;
    }
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
        fprintf(2, "find: path too long\n");
        return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/'; //p指向最后一个'/'之后
    while (read(fd, &de, sizeof de) == sizeof de) {
        if (de.inum == 0) {
            continue;
        }
        memmove(p, de.name, DIRSIZ); // 添加路径名称
        p[DIRSIZ] = 0; // 字符串结束标志
        // stat函数接收一个路径作为参数，将该路径映射到stat结构体
        if (stat(buf, &st) < 0) {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }
        // 不在"."和".."中递归
        if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) {
            find(buf, filename);
        }
        else if (strcmp(filename, p) == 0) {
            printf("%s\n", buf);
        }
    }

    close(fd);
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(2, "usage: find <direcory> <filename>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}