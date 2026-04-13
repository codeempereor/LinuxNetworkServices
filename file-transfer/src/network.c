/*****************************************************************
 > File Name:    network.c
 > Author:       三道渊
 > Description:  网络传输实现
 > Created Time: 2026年04月13日 星期日 19时30分00秒
 *****************************************************************/

#include "network.h"

/**
 * @brief 创建服务器套接字
 * @param port 端口号
 * @return 服务器套接字
 */
int create_server_socket(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        error_exit("socket creation failed");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        error_exit("setsockopt failed");
    }

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error_exit("bind failed");
    }

    if (listen(server_fd, 5) < 0) {
        error_exit("listen failed");
    }

    return server_fd;
}

/**
 * @brief 接受客户端连接
 * @param server_fd 服务器套接字
 * @param client_addr 客户端地址
 * @return 客户端套接字
 */
int accept_client(int server_fd, struct sockaddr_in *client_addr) {
    socklen_t client_addr_len = sizeof(*client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)client_addr, &client_addr_len);
    if (client_fd < 0) {
        error_exit("accept failed");
    }
    return client_fd;
}

/**
 * @brief 创建客户端套接字
 * @param server_ip 服务器IP
 * @param port 端口号
 * @return 客户端套接字
 */
int create_client_socket(const char *server_ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error_exit("socket creation failed");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        error_exit("inet_pton failed");
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error_exit("connect failed");
    }

    return sockfd;
}

/**
 * @brief 发送数据
 * @param sockfd 套接字
 * @param data 数据
 * @param size 数据大小
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int send_data(int sockfd, const void *data, size_t size) {
    size_t sent = 0;
    while (sent < size) {
        ssize_t n = send(sockfd, (const char *)data + sent, size - sent, 0);
        if (n < 0) {
            return ERROR_NETWORK;
        }
        sent += n;
    }
    return ERROR_SUCCESS;
}

/**
 * @brief 接收数据
 * @param sockfd 套接字
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int recv_data(int sockfd, void *buffer, size_t size) {
    size_t received = 0;
    while (received < size) {
        ssize_t n = recv(sockfd, (char *)buffer + received, size - received, 0);
        if (n < 0) {
            return ERROR_NETWORK;
        } else if (n == 0) {
            return ERROR_NETWORK;
        }
        received += n;
    }
    return ERROR_SUCCESS;
}

/**
 * @brief 发送命令
 * @param sockfd 套接字
 * @param command 命令
 * @param ... 可变参数
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int send_command(int sockfd, const char *command, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, command);
    vsnprintf(buffer, sizeof(buffer), command, args);
    va_end(args);
    strcat(buffer, "\n");
    return send_data(sockfd, buffer, strlen(buffer));
}

/**
 * @brief 接收命令
 * @param sockfd 套接字
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int recv_command(int sockfd, char *buffer, size_t size) {
    char *pos;
    size_t received = 0;

    while (received < size - 1) {
        ssize_t n = recv(sockfd, buffer + received, 1, 0);
        if (n < 0) {
            return ERROR_NETWORK;
        } else if (n == 0) {
            return ERROR_NETWORK;
        }

        received++;
        buffer[received] = '\0';

        if ((pos = strchr(buffer, '\n')) != NULL) {
            *pos = '\0';
            break;
        }
    }

    return ERROR_SUCCESS;
}
