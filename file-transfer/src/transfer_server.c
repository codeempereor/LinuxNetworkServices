/*****************************************************************
 > File Name:    server.c
 > Author:       三道渊
 > Description:  服务器端实现
 > Created Time: 2026年04月13日 星期日 19时30分00秒
 *****************************************************************/

#include "common.h"
#include "network.h"
#include "database.h"
#include "thread_pool.h"

/**
 * @brief 处理客户端连接
 * @param client_fd 客户端套接字
 * @param client_ip 客户端IP
 */
void handle_client(int client_fd, const char *client_ip) {
    char buffer[1024];
    while (1) {
        // 接收客户端命令
        if (recv_command(client_fd, buffer, sizeof(buffer)) != ERROR_SUCCESS) {
            break;
        }

        // 处理CHECK_UPLOAD命令
        if (strncmp(buffer, CMD_CHECK_UPLOAD, strlen(CMD_CHECK_UPLOAD)) == 0) {
            char filename[MAX_FILENAME_LENGTH];
            sscanf(buffer + strlen(CMD_CHECK_UPLOAD) + 1, "%s", filename);

            // 连接数据库
            MYSQL *mysql = db_connect();
            if (mysql == NULL) {
                send_command(client_fd, "ERROR: Database connection failed");
                continue;
            }

            // 查询任务
            TransferTask task;
            strcpy(task.filename, filename);
            int ret = db_get_task(mysql, filename, &task);
            if (ret != ERROR_SUCCESS) {
                send_command(client_fd, "ERROR: Database query failed");
                db_disconnect(mysql);
                continue;
            }

            // 发送进度信息
            if (task.id == 0) {
                // 文件不存在
                send_command(client_fd, "PROGRESS: 0 0");
            } else {
                send_command(client_fd, "PROGRESS: %d %d", task.total_blocks, task.completed_blocks);
            }

            db_disconnect(mysql);
        }
        // 处理CHECK_DOWNLOAD命令
        else if (strncmp(buffer, CMD_CHECK_DOWNLOAD, strlen(CMD_CHECK_DOWNLOAD)) == 0) {
            char filename[MAX_FILENAME_LENGTH];
            sscanf(buffer + strlen(CMD_CHECK_DOWNLOAD) + 1, "%s", filename);

            // 连接数据库
            MYSQL *mysql = db_connect();
            if (mysql == NULL) {
                send_command(client_fd, "ERROR: Database connection failed");
                continue;
            }

            // 查询任务
            TransferTask task;
            strcpy(task.filename, filename);
            int ret = db_get_task(mysql, filename, &task);
            if (ret != ERROR_SUCCESS) {
                send_command(client_fd, "ERROR: Database query failed");
                db_disconnect(mysql);
                continue;
            }

            // 发送文件信息
            if (task.id == 0 || task.status != STATUS_COMPLETED) {
                send_command(client_fd, "ERROR: File not found or not completed");
            } else {
                send_command(client_fd, "PROGRESS: %d %lld", task.total_blocks, task.file_size);
            }

            db_disconnect(mysql);
        }
        // 处理QUERY_PROGRESS命令
        else if (strncmp(buffer, CMD_QUERY_PROGRESS, strlen(CMD_QUERY_PROGRESS)) == 0) {
            char filename[MAX_FILENAME_LENGTH];
            sscanf(buffer + strlen(CMD_QUERY_PROGRESS) + 1, "%s", filename);

            // 连接数据库
            MYSQL *mysql = db_connect();
            if (mysql == NULL) {
                send_command(client_fd, "ERROR: Database connection failed");
                continue;
            }

            // 查询任务
            TransferTask task;
            strcpy(task.filename, filename);
            int ret = db_get_task(mysql, filename, &task);
            if (ret != ERROR_SUCCESS) {
                send_command(client_fd, "ERROR: Database query failed");
                db_disconnect(mysql);
                continue;
            }

            // 发送进度信息
            if (task.id == 0) {
                send_command(client_fd, "ERROR: File not found");
            } else {
                int percentage = task.total_blocks > 0 ? (task.completed_blocks * 100) / task.total_blocks : 0;
                send_command(client_fd, "PROGRESS: %d%% (%d/%d blocks)", percentage, task.completed_blocks, task.total_blocks);
            }

            db_disconnect(mysql);
        }
        // 处理BLOCK命令
        else if (strncmp(buffer, CMD_BLOCK, strlen(CMD_BLOCK)) == 0) {
            // 解析分块信息
            BlockInfo block;
            sscanf(buffer + strlen(CMD_BLOCK) + 1, "%d %d %lld %s", &block.block_id, &block.total_blocks, &block.file_size, block.md5);

            // 接收分块数据
            if (recv_data(client_fd, block.data, BLOCK_SIZE) != ERROR_SUCCESS) {
                break;
            }

            // 处理分块数据
            // 这里需要实现分块数据的写入和进度更新
            // 暂时留空，后续完善
        }
        else {
            send_command(client_fd, "ERROR: Unknown command");
        }
    }

    close(client_fd);
}

/**
 * @brief 主函数
 * @return 0
 */
int main() {
    // 创建服务器套接字
    int server_fd = create_server_socket(SERVER_PORT);
    printf("Server listening on port %d\n", SERVER_PORT);

    // 创建线程池
    ThreadPool *pool = thread_pool_create(THREAD_POOL_INIT_SIZE, THREAD_POOL_MAX_SIZE, TASK_QUEUE_CAPACITY);

    while (1) {
        // 接受客户端连接
        struct sockaddr_in client_addr;
        int client_fd = accept_client(server_fd, &client_addr);
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Client connected: %s\n", client_ip);

        // 处理客户端连接
        handle_client(client_fd, client_ip);
    }

    // 销毁线程池
    thread_pool_destroy(pool);
    close(server_fd);

    return 0;
}
