/*****************************************************************
 > File Name:    search_server.c
 > Author:       三道渊
 > Description:  检索服务主程序，实现守护进程、多线程并发处理
 > Created Time: 2026年03月03日 星期六 13时01分11秒
 *****************************************************************

#include "../include/common.h"

/**
 * @brief 清理过期记录线程
 * @param arg 线程参数（未使用）
 * @return NULL
 * @return NULL
 * @description 每天执行一次，清理数据库中过期的搜索记录
 */
void* cleanup_thread(void *arg)
{
    (void)arg;  /* 未使用的参数 */
    
    while (1)
    {
        sleep(24 * 3600);  /* 每天执行一次 */
        db_clean_old_records();
    }
    
    return NULL;
}

/**
 * @brief 主函数
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 程序退出状态码
 * @description 服务器主入口，负责初始化系统、创建线程池、处理客户端连接
 */
int main(int argc, char *argv[])
{
    int daemon_mode = 1;  /* 默认以守护进程模式运行 */
    
    /**
     * 解析命令行参数
     * -f, --foreground: 前台模式运行，不创建守护进程
     */
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--foreground") == 0)
        {
            daemon_mode = 0;  /* 前台模式运行 */
        }
    }
    
    /* 创建守护进程 */
    if (daemon_mode)
    {
        if (daemonize() < 0)
        {
            fprintf(stderr, "Failed to daemonize\n");
            return 1;
        }
    }
    
    /**
     * @brief 信号处理函数
     * @param signum 信号编号
     * @description 处理SIGTERM、SIGINT、SIGUSR1信号，分别对应终止、中断、用户自定义信号
     */
    /* 设置信号处理 */
    signal(SIGTERM, signal_handler);  /* 终止信号 */
    signal(SIGINT, signal_handler);    /* 中断信号（Ctrl+C） */
    signal(SIGUSR1, signal_handler);   /* 用户自定义信号 */
    
    /**
     * 初始化数据库
     * 连接数据库、创建必要的表（如果不存在）
     */
    if (db_init() < 0)
    {
        write_log("Failed to initialize database");
        return 1;
    }
    
    /**
     * @brief 构建初始文件索引
     * @description 遍历搜索目录，递归构建文件索引二叉树
     * @return 索引中包含的文件数量
     */
    write_log("Building initial file index...");
    int file_count = build_file_index(SEARCH_BASE_DIR);
    write_log("Initial index built: %d files", file_count);
    
    /**
     * 将索引插入数据库
     * 加锁保护共享资源，确保线程安全
     */
    pthread_mutex_lock(&g_index_mutex);  /* 加锁保护共享资源 */
    if (g_file_index_root != NULL)
    {
        db_insert_file_index(g_file_index_root);
    }
    pthread_mutex_unlock(&g_index_mutex);  /* 解锁 */
    
    /**
     * 创建索引更新线程
     * 负责定时将内存中的索引更新到数据库
     */
    pthread_t update_tid;
    if (pthread_create(&update_tid, NULL, index_update_thread, NULL) != 0)
    {
        write_log("Failed to create index update thread");
        db_close();
        return 1;
    }
    pthread_detach(update_tid);  /* 分离线程，自动回收资源 */
    
    /**
     * 创建清理线程
     * 每天执行一次，清理数据库中过期的搜索记录
     */
    pthread_t cleanup_tid;
    if (pthread_create(&cleanup_tid, NULL, cleanup_thread, NULL) != 0)
    {
        write_log("Failed to create cleanup thread");
    }
    pthread_detach(cleanup_tid);  /* 分离线程 */
    
    /**
     * @brief 初始化TCP服务端
     * @return 成功返回监听socket，失败返回-1
     * @description 启动TCP服务端监听指定端口，准备接受客户端连接
     */
    int server_sock = server_init();
    if (server_sock < 0)
    {
        write_log("Failed to initialize server");
        db_close();
        return 1;
    }
    
    write_log("Server started successfully");
    
    /**
     * @brief 主循环：接受客户端连接
     * @description 服务器主循环，负责接受客户端连接、创建线程处理请求
     * @return NULL
     */
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0)
        {
            write_log("accept failed");
            continue;
        }
        
        /**
         * @brief 创建客户端处理线程
         * @param client_sock_ptr 客户端socket指针
         * @return NULL
         * @description 启动新线程处理客户端请求，释放当前线程资源
         */
        /* 创建客户端处理线程
         * @param client_sock_ptr 客户端socket指针
         * @return NULL
         * @description 启动新线程处理客户端请求，释放当前线程资源
         */
        int *client_sock_ptr = malloc(sizeof(int));
        if (client_sock_ptr == NULL)
        {
            close(client_sock);
            continue;
        }
        *client_sock_ptr = client_sock;
        
        /**
         * @brief 创建客户端处理线程
         * @param client_sock_ptr 指向客户端socket的指针
         * @description 为每个客户端连接创建一个线程，负责处理该连接的请求
         */
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, client_sock_ptr) != 0)
        {
            free(client_sock_ptr);
            close(client_sock);
            write_log("Failed to create client handler thread");
            continue;
        }
        pthread_detach(tid);  /* 分离线程 */
    }
    
        /**
         * @brief 清理资源（正常情况下不会执行到这里）
         * @description 关闭服务器socket、数据库连接、释放文件索引树内存
         */
    close(server_sock);
    db_close();
    if (g_file_index_root != NULL)
    {
        free_tree(g_file_index_root);
    }
    
    return 0;
}
