#include "kernel/types.h"
#include "user/user.h"

#define RD 0 // pipe的读端
#define WR 1 // pipe的写端

int main(int argc, char const *argv[]) 
{
    char buf = 'P'; // 用于传递的字节

    int fd_c2p[2]; // 子进程->父进程
    int fd_p2c[2]; // 父进程->子进程
    pipe(fd_c2p);
    pipe(fd_p2c);

    int pid = fork();
    int exit_status = 0;

    if (pid < 0) {
        fprintf(2, "fork() error!\n");
        close(fd_c2p[RD]);
        close(fd_c2p[WR]);
        close(fd_p2c[RD]);
        close(fd_p2c[WR]);
        exit(1);
    }
    else if (pid == 0) { // 子进程
        close(fd_p2c[WR]); // 关闭父子管道写端
        close(fd_c2p[RD]); // 关闭子父管道读端
        // 读入父子管道传入字节
        if (read(fd_p2c[RD], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "child read() error!\n");
            exit_status = 1; 
        }
        else {
            fprintf(1, "%d: received ping\n", getpid());
        }

        // 向子父管道写入字节 
        if (write(fd_c2p[WR], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "child write() error!\n");
            exit_status = 1;
        }

        close(fd_p2c[RD]);
        close(fd_c2p[WR]);

        exit(exit_status);
    }
    else { // 父进程
        close(fd_p2c[RD]);
        close(fd_c2p[WR]);

        if (write(fd_p2c[WR], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "parent write error!\n");
            exit_status = 1;
        }

        if (read(fd_c2p[RD], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "parent read error!\n");
            exit_status = 1;
        }
        else {
            fprintf(1, "%d: received pong\n", getpid());
        }

        close(fd_p2c[WR]);
        close(fd_c2p[RD]);

        exit(exit_status);
    }
}