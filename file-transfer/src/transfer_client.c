/*****************************************************************
 > File Name:    client.c
 > Author:       三道渊
 > Description:  客户端实现
 > Created Time: 2026年04月13日 星期日 19时30分00秒
 *****************************************************************/

#include "common.h"
#include "network.h"

/**
 * @brief 显示进度条
 * @param completed 已完成分块数
 * @param total 总分块数
 */
void show_progress(int completed, int total) {
    int percentage = total > 0 ? (completed * 100) / total : 0;
    int bar_length = 20;
    int filled_length = (percentage * bar_length) / 100;

    printf("[");
    for (int i = 0; i < bar_length; i++) {
        if (i < filled_length) {
            printf("=");
        } else {
            printf(" ");
        }
    }
    printf("] %d%%\r", percentage);
    fflush(stdout);
}

/**
 * @brief 上传文件
 * @param local_file 本地文件路径
 * @param server_ip 服务器IP
 * @param port 端口号
 */
void upload_file(const char *local_file, const char *server_ip, int port) {
    // 创建客户端套接字
    int sockfd = create_client_socket(server_ip, port);

    // 获取文件名
    char filename[MAX_FILENAME_LENGTH];
    char *last_slash = strrchr(local_file, '/');
    if (last_slash != NULL) {
        strcpy(filename, last_slash + 1);
    } else {
        strcpy(filename, local_file);
    }

    // 发送CHECK_UPLOAD命令
    send_command(sockfd, "CHECK_UPLOAD %s", filename);

    // 接收服务器响应
    char buffer[1024];
    recv_command(sockfd, buffer, sizeof(buffer));

    // 解析响应
    int total_blocks, completed_blocks;
    if (sscanf(buffer, "PROGRESS: %d %d", &total_blocks, &completed_blocks) != 2) {
        printf("Error: Invalid response from server\n");
        close(sockfd);
        return;
    }

    // 获取文件大小
    long long file_size = get_file_size(local_file);
    if (file_size < 0) {
        printf("Error: Failed to get file size\n");
        close(sockfd);
        return;
    }

    // 计算总分块数
    if (total_blocks == 0) {
        total_blocks = calculate_total_blocks(file_size);
    }

    // 计算文件MD5
    char *file_md5 = calculate_md5(local_file);
    if (file_md5 == NULL) {
        printf("Error: Failed to calculate MD5\n");
        close(sockfd);
        return;
    }

    // 打开文件
    int fd = open(local_file, O_RDONLY);
    if (fd < 0) {
        printf("Error: Failed to open file\n");
        free(file_md5);
        close(sockfd);
        return;
    }

    // 上传分块
    for (int i = completed_blocks; i < total_blocks; i++) {
        // 计算分块偏移量
        off_t offset = (off_t)i * BLOCK_SIZE;
        lseek(fd, offset, SEEK_SET);

        // 读取分块数据
        BlockInfo block;
        block.block_id = i;
        block.total_blocks = total_blocks;
        block.file_size = file_size;
        strcpy(block.md5, file_md5);

        ssize_t read_size = read(fd, block.data, BLOCK_SIZE);
        if (read_size < 0) {
            printf("Error: Failed to read file\n");
            free(file_md5);
            close(fd);
            close(sockfd);
            return;
        }
        block.data_size = read_size;

        // 发送分块头
        send_command(sockfd, "BLOCK: %d %d %lld %s", block.block_id, block.total_blocks, block.file_size, block.md5);

        // 发送分块数据
        send_data(sockfd, block.data, block.data_size);

        // 显示进度
        show_progress(i + 1, total_blocks);
    }

    // 接收传输结果
    recv_command(sockfd, buffer, sizeof(buffer));
    printf("\n%s\n", buffer);

    // 清理资源
    free(file_md5);
    close(fd);
    close(sockfd);
}

/**
 * @brief 下载文件
 * @param remote_file 远程文件名
 * @param local_path 本地路径
 * @param server_ip 服务器IP
 * @param port 端口号
 */
void download_file(const char *remote_file, const char *local_path, const char *server_ip, int port) {
    // 创建客户端套接字
    int sockfd = create_client_socket(server_ip, port);

    // 发送CHECK_DOWNLOAD命令
    send_command(sockfd, "CHECK_DOWNLOAD %s", remote_file);

    // 接收服务器响应
    char buffer[1024];
    recv_command(sockfd, buffer, sizeof(buffer));

    // 解析响应
    int total_blocks;
    long long file_size;
    if (sscanf(buffer, "PROGRESS: %d %lld", &total_blocks, &file_size) != 2) {
        printf("Error: Invalid response from server\n");
        close(sockfd);
        return;
    }

    // 构建本地文件路径
    char local_file[MAX_FILENAME_LENGTH];
    snprintf(local_file, sizeof(local_file), "%s/%s", local_path, remote_file);

    // 打开文件
    int fd = open(local_file, O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        printf("Error: Failed to open file\n");
        close(sockfd);
        return;
    }

    // 下载分块
    for (int i = 0; i < total_blocks; i++) {
        // 发送分块请求
        send_command(sockfd, "BLOCK: %d", i);

        // 接收分块数据
        BlockInfo block;
        if (recv_data(sockfd, &block, sizeof(BlockInfo)) != ERROR_SUCCESS) {
            printf("Error: Failed to receive block\n");
            close(fd);
            close(sockfd);
            return;
        }

        // 写入分块数据
        lseek(fd, (off_t)i * BLOCK_SIZE, SEEK_SET);
        write(fd, block.data, block.data_size);

        // 显示进度
        show_progress(i + 1, total_blocks);
    }

    // 接收传输结果
    recv_command(sockfd, buffer, sizeof(buffer));
    printf("\n%s\n", buffer);

    // 清理资源
    close(fd);
    close(sockfd);
}

/**
 * @brief 查询进度
 * @param remote_file 远程文件名
 * @param server_ip 服务器IP
 * @param port 端口号
 */
void query_progress(const char *remote_file, const char *server_ip, int port) {
    // 创建客户端套接字
    int sockfd = create_client_socket(server_ip, port);

    // 发送QUERY_PROGRESS命令
    send_command(sockfd, "QUERY_PROGRESS %s", remote_file);

    // 接收服务器响应
    char buffer[1024];
    recv_command(sockfd, buffer, sizeof(buffer));

    // 显示进度
    printf("%s\n", buffer);

    // 清理资源
    close(sockfd);
}

/**
 * @brief 主函数
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 0
 */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("  Upload: %s upload local_file server_ip:port\n", argv[0]);
        printf("  Download: %s download remote_file local_path server_ip:port\n", argv[0]);
        printf("  Query: %s query remote_file server_ip:port\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "upload") == 0) {
        if (argc != 4) {
            printf("Usage: %s upload local_file server_ip:port\n", argv[0]);
            return 1;
        }

        char *local_file = argv[2];
        char *server_addr = argv[3];

        // 解析服务器地址
        char server_ip[INET_ADDRSTRLEN];
        int port;
        if (sscanf(server_addr, "%[^:]:%d", server_ip, &port) != 2) {
            port = SERVER_PORT;
            strcpy(server_ip, server_addr);
        }

        upload_file(local_file, server_ip, port);
    }
    else if (strcmp(argv[1], "download") == 0) {
        if (argc != 5) {
            printf("Usage: %s download remote_file local_path server_ip:port\n", argv[0]);
            return 1;
        }

        char *remote_file = argv[2];
        char *local_path = argv[3];
        char *server_addr = argv[4];

        // 解析服务器地址
        char server_ip[INET_ADDRSTRLEN];
        int port;
        if (sscanf(server_addr, "%[^:]:%d", server_ip, &port) != 2) {
            port = SERVER_PORT;
            strcpy(server_ip, server_addr);
        }

        download_file(remote_file, local_path, server_ip, port);
    }
    else if (strcmp(argv[1], "query") == 0) {
        if (argc != 4) {
            printf("Usage: %s query remote_file server_ip:port\n", argv[0]);
            return 1;
        }

        char *remote_file = argv[2];
        char *server_addr = argv[3];

        // 解析服务器地址
        char server_ip[INET_ADDRSTRLEN];
        int port;
        if (sscanf(server_addr, "%[^:]:%d", server_ip, &port) != 2) {
            port = SERVER_PORT;
            strcpy(server_ip, server_addr);
        }

        query_progress(remote_file, server_ip, port);
    }
    else {
        printf("Invalid operation: %s\n", argv[1]);
        return 1;
    }

    return 0;
}