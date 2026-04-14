/*****************************************************************
 > File Name:    server.h
 > Author:       三道渊
 > Description:  服务端头文件，定义服务端相关函数
 > Created Time: 2026年04月14日 星期一 16时59分00秒
 *****************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include "common.h"

/**
 * 服务端结构体
 */
typedef struct {
    int tcp_sockfd;        // TCP套接字文件描述符
    int udp_sockfd;        // UDP套接字文件描述符
    struct sockaddr_in tcp_addr; // TCP地址
    struct sockaddr_in udp_addr; // UDP地址
    ServerConfig config;   // 服务端配置
    LogQueue* queue;       // 日志队列
    pthread_t tcp_thread;  // TCP接收线程
    pthread_t udp_thread;  // UDP接收线程
    pthread_t* consumer_threads; // 消费者线程
    int running;           // 运行状态
    int filter_levels[4];  // 过滤级别
    pthread_mutex_t filter_mutex; // 过滤互斥锁
} LogServer;

/**
 * 初始化服务端
 * @param config_file 配置文件路径
 * @return 服务端实例
 */
LogServer* server_init(const char* config_file);

/**
 * 启动服务端
 * @param server 服务端实例
 * @return 错误码
 */
int server_start(LogServer* server);

/**
 * 停止服务端
 * @param server 服务端实例
 */
void server_stop(LogServer* server);

/**
 * 清理服务端
 * @param server 服务端实例
 */
void server_cleanup(LogServer* server);

/**
 * 解析配置文件
 * @param config_file 配置文件路径
 * @param config 配置结构体
 * @return 错误码
 */
int parse_server_config(const char* config_file, ServerConfig* config);

/**
 * 设置日志过滤
 * @param server 服务端实例
 * @param levels 过滤级别数组
 * @param count 级别数量
 */
void set_log_filter(LogServer* server, int* levels, int count);

/**
 * 取消日志过滤
 * @param server 服务端实例
 */
void clear_log_filter(LogServer* server);

/**
 * 导出日志
 * @param start_time 开始时间
 * @param end_time 结束时间
 * @param level 日志级别
 * @return 错误码
 */
int export_logs(time_t start_time, time_t end_time, log_level level);

#endif /* SERVER_H */