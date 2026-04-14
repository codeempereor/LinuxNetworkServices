/*****************************************************************
 > File Name:    client.h
 > Author:       三道渊
 > Description:  客户端头文件，定义客户端相关函数
 > Created Time: 2026年04月14日 星期一 16时58分00秒
 *****************************************************************/

#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

/**
 * 客户端结构体
 */
typedef struct {
    int sockfd;            // 套接字文件描述符
    struct sockaddr_in server_addr; // 服务器地址
    ClientConfig config;   // 客户端配置
    LogEntry cache[MAX_CACHE_SIZE]; // 日志缓存
    int cache_size;        // 缓存大小
    int cache_head;        // 缓存头部
    int cache_tail;        // 缓存尾部
    pthread_mutex_t cache_mutex; // 缓存互斥锁
} LogClient;

/**
 * 初始化客户端
 * @param config_file 配置文件路径
 * @return 客户端实例
 */
LogClient* client_init(const char* config_file);

/**
 * 发送日志
 * @param client 客户端实例
 * @param level 日志级别
 * @param module 模块名
 * @param content 日志内容
 * @return 错误码
 */
int log_send(LogClient* client, log_level level, const char* module, const char* content);

/**
 * 重发缓存的日志
 * @param client 客户端实例
 * @return 成功重发的日志数量
 */
int resend_cache(LogClient* client);

/**
 * 清理客户端
 * @param client 客户端实例
 */
void client_cleanup(LogClient* client);

/**
 * 解析配置文件
 * @param config_file 配置文件路径
 * @param config 配置结构体
 * @return 错误码
 */
int parse_client_config(const char* config_file, ClientConfig* config);

#endif /* CLIENT_H */