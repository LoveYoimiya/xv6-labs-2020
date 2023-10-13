#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXSZ 512
// 有限状态自动机状态定义
enum state {
    S_WAIT,         // 等待参数输入
    S_ARG,          // 参数内
    S_ARG_END,      // 参数结束
    S_ARG_LINE_END, // 左侧有参数的换行，e.g."arg\n"
    S_LINE_END,     // 左侧为空格的换行，e.g."arg \n"
    S_END           // 结束，EOF
};

// 字符类型定义
enum char_type {
    C_SPACE,
    C_CHAR,
    C_LINE_END
};
/**
 * @brief 获取字符类型
 * @param c 输入字符
 * @return enum char_type 字符类型
*/
enum char_type get_char_type(char c) {
    switch (c) {
    case ' ' : 
        return C_SPACE;
    case '\n' :
        return C_LINE_END;
    default : 
        return C_CHAR;
    }
}
/**
 * @brief 状态转换函数
 * @param cur 当前状态
 * @param ct 将要读取的字符
 * @return enum state 转换后的状态
*/
enum state transform_state(enum state cur, enum char_type ct) {
    switch (cur) {
    case S_WAIT :
        if (ct == C_SPACE)      return S_WAIT;
        if (ct == C_LINE_END)   return S_LINE_END;
        if (ct == C_CHAR)       return S_ARG;
        break;
    case S_ARG :
        if (ct == C_SPACE)      return S_ARG_END;
        if (ct == C_LINE_END)   return S_ARG_LINE_END;
        if (ct == C_CHAR)       return S_ARG;
        break;
    case S_ARG_END :
    case S_ARG_LINE_END :
    case S_LINE_END :
        if (ct == C_SPACE)      return S_WAIT;
        if (ct == C_LINE_END)   return S_LINE_END;
        if (ct == C_CHAR)       return S_ARG;
        break;
    default :
        break;
    }
    return S_END;
}
/**
 * @brief 将参数列表后面的元素全部置为空，用于换行时，重新赋予参数
 * @param x_argv 参数指针数组
 * @param beg 要清空的起始下标
*/
void clearArgv(char* x_argv[MAXARG], int beg) {
    for (int i = beg; i < MAXARG; i++) {
        x_argv[i] = 0;
    }
}

int main(int argc, char* argv[]) 
{
    if (argc - 1 > MAXARG) {
        fprintf(2, "xargs: too many arguments.\n");
        exit(1);
    }
    char lines[MAXSZ];
    char* p = lines;
    char* x_argv[MAXARG] = {0}; // 参数指针函数， 全部初始化为空指针

    // 存储原有的参数
    for (int i = 1; i < argc; i++) {
        x_argv[i - 1] = argv[i];
    }
    int arg_beg = 0;
    int arg_end = 0;
    int arg_cnt = argc - 1;
    enum state st = S_WAIT;

    while (st != S_END) {
        if (read(0, p, sizeof(char)) != sizeof(char)) { // 读取为空则退出
            st = S_END;
        }
        else {
            st = transform_state(st, get_char_type(*p));
        }
        // printf("p: %s\n", *p);
        // printf("arg_end: %d\n", arg_end);
        if (++arg_end >= MAXSZ) {
            fprintf(2, "xargs: arguments too long.\n");
            exit(1);
        }

        switch (st) {
        case S_WAIT :
            ++ arg_beg;
            break;
        case S_ARG_END :            // 参数结束，将参数地址存入x_argv数组中
            x_argv[arg_cnt++] = &lines[arg_beg];
            arg_beg = arg_end;
            *p = '\0';              // 替换为字符串结束符
            break;
        case S_ARG_LINE_END :       // 将参数地址存入x_argv数组中同时执行指令
            x_argv[arg_cnt++] = &lines[arg_beg];
            // 不加break
        case S_LINE_END :           // 行结束，则为当前行执行指令
            arg_beg = arg_end;
            *p = '\0';
            if (fork() == 0) {
                exec(argv[1], x_argv);
            }
            arg_cnt = argc - 1;
            clearArgv(x_argv, arg_cnt);
            wait(0);
            break;
        default :
            break;
        }

        ++p;
    }
    exit(0);
}