/*****************************************************************
 > File Name:    common.h
 > Author:       三道渊
 > Description:  公共头文件，定义常量、枚举和结构体
 > Created Time: 2026年04月14日 星期一 16时57分00秒
 *****************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

/**
 * 日志级别枚举
 */
typedef enum {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
} log_level;

/**
 * 协议类型
 */
typedef enum {
    PROTOCOL_TCP = 0,
    PROTOCOL_UDP = 1
} protocol_type;

/**
 * 日志结构体
 */
typedef struct {
    time_t timestamp;      // 时间戳
    char ip[16];           // 客户端IP
    char module[64];       // 模块名
    log_level level;       // 日志级别
    char content[1024];    // 日志内容
} LogEntry;

/**
 * 队列节点结构体
 */
typedef struct QueueNode {
    LogEntry log;          // 日志条目
    struct QueueNode *next; // 下一个节点
} QueueNode;

/**
 * 队列结构体
 */
typedef struct {
    QueueNode *head;       // 队首
    QueueNode *tail;       // 队尾
    int size;              // 队列大小
    int capacity;          // 队列容量
    pthread_mutex_t mutex;  // 互斥锁
    pthread_cond_t not_full; // 非满条件变量
    pthread_cond_t not_empty; // 非空条件变量
} LogQueue;

/**
 * 配置结构体
 */
typedef struct {
    protocol_type protocol; // 协议类型
    char server_ip[16];     // 服务器IP
    int server_port;        // 服务器端口
    int max_cache;          // 最大缓存数量
} ClientConfig;

/**
 * 服务端配置结构体
 */
typedef struct {
    int client1_port;       // 客户端1端口
    int client2_port;       // 客户端2端口
    int queue_capacity;     // 队列容量
    int consumer_threads;   // 消费者线程数
} ServerConfig;

/**
 * 全局常量
 */
#define MAX_LOG_SIZE 2048          // 最大日志大小
#define MAX_CACHE_SIZE 100         // 客户端最大缓存数量
#define MAX_QUEUE_SIZE 1000        // 服务端队列最大容量
#define SERVER_PORT 8888           // 默认服务端端口
#define CLIENT_BUFFER_SIZE 4096    // 客户端缓冲区大小
#define SERVER_BUFFER_SIZE 4096    // 服务端缓冲区大小

/**
 * 日志级别字符串
 */
static const char* log_level_str[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

/**
 * 协议类型字符串
 */
// static const char* protocol_str[] = {
//     "TCP",
//     "UDP"
// };

/**
 * 错误码
 */
#define ERROR_SUCCESS 0
#define ERROR_SOCKET -1
#define ERROR_CONNECT -2
#define ERROR_SEND -3
#define ERROR_RECV -4
#define ERROR_DATABASE -5
#define ERROR_CONFIG -6

#endif /* COMMON_H */