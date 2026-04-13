/*****************************************************************
 > File Name:    network.h
 > Author:       三道渊
 > Description:  网络传输相关头文件
 > Created Time: 2026年04月13日 星期日 19时30分00秒
 *****************************************************************/

#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"

/**
 * 函数声明
 */
/**
 * @brief 创建服务器套接字
 * @param port 端口号
 * @return 服务器套接字
 */
int create_server_socket(int port);

/**
 * @brief 接受客户端连接
 * @param server_fd 服务器套接字
 * @param client_addr 客户端地址
 * @return 客户端套接字
 */
int accept_client(int server_fd, struct sockaddr_in *client_addr);

/**
 * @brief 创建客户端套接字
 * @param server_ip 服务器IP
 * @param port 端口号
 * @return 客户端套接字
 */
int create_client_socket(const char *server_ip, int port);

/**
 * @brief 发送数据
 * @param sockfd 套接字
 * @param data 数据
 * @param size 数据大小
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int send_data(int sockfd, const void *data, size_t size);

/**
 * @brief 接收数据
 * @param sockfd 套接字
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int recv_data(int sockfd, void *buffer, size_t size);

/**
 * @brief 发送命令
 * @param sockfd 套接字
 * @param command 命令
 * @param ... 可变参数
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int send_command(int sockfd, const char *command, ...);

/**
 * @brief 接收命令
 * @param sockfd 套接字
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int recv_command(int sockfd, char *buffer, size_t size);

#endif // NETWORK_H
