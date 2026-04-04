/*****************************************************************
 > File Name:    utils.c
 > Author:       三道渊
 > Description:  工具函数模块，实现日志记录、守护进程等功能
 > Created Time: 2026年03月03日 星期六 13时01分11秒
 *****************************************************************/

#include "../include/common.h"
#include <stdarg.h>
#include <fcntl.h>

/**
 * @brief 写入日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void write_log(const char *format, ...)
{
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL)
    {
        return;
    }
    
    /* 获取当前时间 */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    /* 写入时间戳 */
    fprintf(log_file, "[%s] ", time_str);
    
    /* 写入日志内容 */
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    fclose(log_file);
}

/**
 * @brief 创建守护进程
 * @return 成功返回0，失败返回-1
 */
int daemonize(void)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        return -1;
    }
    
    if (pid > 0)
    {
        /* 父进程退出 */
        exit(0);
    }
    
    /* 子进程继续 */
    if (setsid() < 0)
    {
        return -1;
    }
    
    /* 忽略SIGHUP信号 */
    signal(SIGHUP, SIG_IGN);
    
    /* 第二次fork，确保进程不是会话组长 */
    pid = fork();
    if (pid < 0)
    {
        return -1;
    }
    
    if (pid > 0)
    {
        /* 第一个子进程退出 */
        exit(0);
    }
    
    /* 第二个子进程继续 */
    /* 更改工作目录 */
    if (chdir("/") < 0) {
        /* 忽略错误，继续执行 */
    }
    
    /* 关闭标准文件描述符 */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    /* 重定向到/dev/null */
    open("/dev/null", O_RDONLY);  /* stdin */
    open("/dev/null", O_RDWR);    /* stdout */
    open("/dev/null", O_RDWR);    /* stderr */
    
    write_log("Daemon started");
    return 0;
}

/**
 * @brief 信号处理函数
 * @param sig 信号编号
 */
void signal_handler(int sig)
{
    switch (sig)
    {
        case SIGTERM:
        case SIGINT:
            write_log("Received signal %d, shutting down...", sig);
            /* 清理资源 */
            db_close();
            if (g_file_index_root != NULL)
            {
                free_tree(g_file_index_root);
                g_file_index_root = NULL;
            }
            exit(0);
            break;
        case SIGUSR1:
            write_log("Received SIGUSR1, reindexing...");
            build_file_index(SEARCH_BASE_DIR);
            break;
        default:
            break;
    }
}
