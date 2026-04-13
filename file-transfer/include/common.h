/*****************************************************************
 > File Name:    common.h
 > Author:       三道渊
 > Description:  公共头文件，定义常量、结构体和函数声明
 > Created Time: 2026年04月13日 星期日 19时30分00秒
 *****************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <openssl/md5.h>

/**
 * 常量定义
 */
/* 分块大小（4MB） */
#define BLOCK_SIZE (4 * 1024 * 1024)
/* 最大文件大小（10GB） */
#define MAX_FILE_SIZE (10 * 1024 * 1024 * 1024)
/* 最大缓冲区大小 */
#define MAX_BUFFER_SIZE (BLOCK_SIZE + 1024)
/* 服务端端口 */
#define SERVER_PORT 8888
/* 最大文件名长度 */
#define MAX_FILENAME_LENGTH 255
/* 最大客户端IP长度 */
#define MAX_CLIENT_IP_LENGTH 50

/**
 * 线程池配置
 */
/* 初始线程数 */
#define THREAD_POOL_INIT_SIZE 5
/* 最大线程数 */
#define THREAD_POOL_MAX_SIZE 10
/* 任务队列容量 */
#define TASK_QUEUE_CAPACITY 20

/**
 * 传输状态
 */
#define STATUS_TRANSFERRING 0 // 传输中
#define STATUS_COMPLETED 1 // 完成
#define STATUS_FAILED 2 // 失败

/**
 * 命令类型
 */
#define CMD_CHECK_UPLOAD "CHECK_UPLOAD"
#define CMD_CHECK_DOWNLOAD "CHECK_DOWNLOAD"
#define CMD_QUERY_PROGRESS "QUERY_PROGRESS"
#define CMD_BLOCK "BLOCK"
#define CMD_PROGRESS "PROGRESS"
#define CMD_TRANSFER_SUCCESS "TRANSFER SUCCESS"
#define CMD_MD5_MISMATCH "MD5 MISMATCH"

/**
 * 错误码
 */
#define ERROR_SUCCESS 0
#define ERROR_FILE_OPEN 1
#define ERROR_NETWORK 2
#define ERROR_DATABASE 3
#define ERROR_MD5 4
#define ERROR_THREAD 5

/**
 * 传输任务结构体
 */
typedef struct {
    int id;
    char filename[MAX_FILENAME_LENGTH];
    long long file_size;
    int total_blocks;
    int completed_blocks;
    char client_ip[MAX_CLIENT_IP_LENGTH];
    int status;
    char md5[33];
} TransferTask;

/**
 * 分块信息结构体
 */
typedef struct {
    int block_id;
    int total_blocks;
    long long file_size;
    char md5[33];
    char data[BLOCK_SIZE];
    int data_size;
} BlockInfo;

/**
 * 线程池任务结构体
 */
typedef struct {
    int client_fd;
    char filename[MAX_FILENAME_LENGTH];
    int operation; // 0: 上传, 1: 下载, 2: 查询进度
} Task;

/**
 * 函数声明
 */
/**
 * @brief 错误处理函数
 * @param msg 错误信息
 */
void error_exit(const char *msg);

/**
 * @brief 安全的内存分配
 * @param size 内存大小
 * @return 分配的内存指针
 */
void *safe_malloc(size_t size);

/**
 * @brief 计算文件的MD5值
 * @param file_path 文件路径
 * @return MD5字符串
 */
char *calculate_md5(const char *file_path);

/**
 * @brief 获取文件大小
 * @param file_path 文件路径
 * @return 文件大小
 */
long long get_file_size(const char *file_path);

/**
 * @brief 计算总分块数
 * @param file_size 文件大小
 * @return 分块数
 */
int calculate_total_blocks(long long file_size);

#endif // COMMON_H
