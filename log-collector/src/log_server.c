/*****************************************************************
 > File Name:    log_server.c
 > Author:       三道渊
 > Description:  服务端实现文件
 > Created Time: 2026年04月14日 星期一 17时03分00秒
 *****************************************************************/

#include "server.h"
#include "queue.h"
#include "database.h"

// 全局停止标志
volatile int g_stop = 0;

/**
 * 主函数
 * @param argc 参数数量
 * @param argv 参数数组
 * @return 退出码
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <config_file>\n", argv[0]);
        return 1;
    }

    // 初始化服务端
    LogServer* server = server_init(argv[1]);
    if (!server) {
        printf("Failed to initialize server\n");
        return 1;
    }

    // 启动服务端
    if (server_start(server) != ERROR_SUCCESS) {
        printf("Failed to start server\n");
        server_cleanup(server);
        return 1;
    }

    printf("Server started successfully\n");
    printf("Commands:\n");
    printf("  filter <level1> <level2> ... - Set log filter\n");
    printf("  filter all - Clear log filter\n");
    printf("  export <level> <start_time> <end_time> - Export logs\n");
    printf("  quit - Exit server\n");

    // 命令处理
    char command[128];
    while (1) {
        printf("Enter command: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "quit") == 0) {
            break;
        } else if (strncmp(command, "filter", 6) == 0) {
            // 处理过滤命令
            char* args = command + 7;
            if (strcmp(args, "all") == 0) {
                clear_log_filter(server);
                printf("Filter cleared\n");
            } else {
                // 解析过滤级别
                int levels[4];
                int count = 0;
                char* token = strtok(args, " ");
                while (token && count < 4) {
                    if (strcmp(token, "DEBUG") == 0) {
                        levels[count++] = DEBUG;
                    } else if (strcmp(token, "INFO") == 0) {
                        levels[count++] = INFO;
                    } else if (strcmp(token, "WARN") == 0) {
                        levels[count++] = WARN;
                    } else if (strcmp(token, "ERROR") == 0) {
                        levels[count++] = ERROR;
                    }
                    token = strtok(NULL, " ");
                }
                if (count > 0) {
                    set_log_filter(server, levels, count);
                    printf("Filter set\n");
                }
            }
        } else if (strncmp(command, "export", 6) == 0) {
            // 处理导出命令
            char* args = command + 7;
            char level_str[10];
            time_t start_time, end_time;
            if (sscanf(args, "%s %ld %ld", level_str, &start_time, &end_time) == 3) {
                log_level level;
                if (strcmp(level_str, "debug") == 0) {
                    level = DEBUG;
                } else if (strcmp(level_str, "info") == 0) {
                    level = INFO;
                } else if (strcmp(level_str, "warn") == 0) {
                    level = WARN;
                } else if (strcmp(level_str, "error") == 0) {
                    level = ERROR;
                } else {
                    printf("Invalid log level\n");
                    continue;
                }
                if (export_logs(start_time, end_time, level) == ERROR_SUCCESS) {
                    printf("Logs exported successfully\n");
                } else {
                    printf("Failed to export logs\n");
                }
            } else {
                printf("Invalid export command\n");
            }
        }
    }

    // 停止服务端
    server_stop(server);
    server_cleanup(server);
    printf("Server stopped\n");
    return 0;
}



/**
 * TCP接收线程函数
 * @param arg 服务端实例
 * @return 线程返回值
 */
void* tcp_receive_thread(void* arg) {
    LogServer* server = (LogServer*)arg;
    int client_sockfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[SERVER_BUFFER_SIZE];

    while (server->running) {
        client_sockfd = accept(server->tcp_sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (client_sockfd < 0) {
            if (server->running) {
                perror("accept failed");
            }
            continue;
        }

        // 接收日志
        while (server->running) {
            int n = recv(client_sockfd, buffer, sizeof(buffer) - 1, 0);
            if (n <= 0) {
                break;
            }
            buffer[n] = '\0';

            // 解析日志
            LogEntry log;
            char level_str[4];
            if (sscanf(buffer, "[%ld][%15s][%63s][%3s]%s",
                       &log.timestamp, log.ip, log.module, level_str, log.content) == 5) {
                // 解析日志级别
                if (strcmp(level_str, "DEBUG") == 0) {
                    log.level = DEBUG;
                } else if (strcmp(level_str, "INFO") == 0) {
                    log.level = INFO;
                } else if (strcmp(level_str, "WARN") == 0) {
                    log.level = WARN;
                } else if (strcmp(level_str, "ERROR") == 0) {
                    log.level = ERROR;
                } else {
                    continue;
                }
                // 检查过滤
                pthread_mutex_lock(&server->filter_mutex);
                int filtered = 1;
                for (int i = 0; i < 4; i++) {
                    if (server->filter_levels[i] && (int)log.level == i) {
                        filtered = 0;
                        break;
                    }
                }
                pthread_mutex_unlock(&server->filter_mutex);

                if (!filtered) {
                    // 入队
                    queue_enqueue(server->queue, &log);
                }
            }
        }

        close(client_sockfd);
    }

    return NULL;
}

/**
 * UDP接收线程函数
 * @param arg 服务端实例
 * @return 线程返回值
 */
void* udp_receive_thread(void* arg) {
    LogServer* server = (LogServer*)arg;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[SERVER_BUFFER_SIZE];

    while (server->running) {
        int n = recvfrom(server->udp_sockfd, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr*)&client_addr, &client_len);
        if (n <= 0) {
            if (server->running) {
                perror("recvfrom failed");
            }
            continue;
        }
        buffer[n] = '\0';

        // 解析日志
        LogEntry log;
        char level_str[4];
        if (sscanf(buffer, "[%ld][%15s][%63s][%3s]%s",
                   &log.timestamp, log.ip, log.module, level_str, log.content) == 5) {
            // 解析日志级别
            if (strcmp(level_str, "DEBUG") == 0) {
                log.level = DEBUG;
            } else if (strcmp(level_str, "INFO") == 0) {
                log.level = INFO;
            } else if (strcmp(level_str, "WARN") == 0) {
                log.level = WARN;
            } else if (strcmp(level_str, "ERROR") == 0) {
                log.level = ERROR;
            } else {
                continue;
            }
            // 检查过滤
            pthread_mutex_lock(&server->filter_mutex);
            int filtered = 1;
            for (int i = 0; i < 4; i++) {
                if (server->filter_levels[i] && (int)log.level == i) {
                    filtered = 0;
                    break;
                }
            }
            pthread_mutex_unlock(&server->filter_mutex);

            if (!filtered) {
                // 入队
                queue_enqueue(server->queue, &log);
            }
        }
    }

    return NULL;
}

/**
 * 消费者线程函数
 * @param arg 服务端实例
 * @return 线程返回值
 */
void* consumer_thread(void* arg) {
    LogServer* server = (LogServer*)arg;
    DBConnection* db = db_init();
    if (!db) {
        printf("Failed to initialize database\n");
        return NULL;
    }

    LogEntry log;
    while (server->running || !queue_is_empty(server->queue)) {
        if (queue_dequeue(server->queue, &log) == ERROR_SUCCESS) {
            // 插入数据库
            db_insert_log(db, &log);
        }
    }

    db_close(db);
    return NULL;
}

/**
 * 信号处理函数
 * @param sig 信号
 */
void signal_handler(int sig) {
    (void)sig; // 避免未使用参数警告
    g_stop = 1;
}

/**
 * 初始化服务端
 * @param config_file 配置文件路径
 * @return 服务端实例
 */
LogServer* server_init(const char* config_file) {
    LogServer* server = (LogServer*)malloc(sizeof(LogServer));
    if (!server) {
        perror("malloc failed");
        return NULL;
    }

    // 初始化配置
    if (parse_server_config(config_file, &server->config) != ERROR_SUCCESS) {
        free(server);
        return NULL;
    }

    // 创建日志队列
    server->queue = queue_create(server->config.queue_capacity);
    if (!server->queue) {
        free(server);
        return NULL;
    }

    // 创建TCP套接字
    server->tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->tcp_sockfd < 0) {
        perror("socket failed");
        queue_destroy(server->queue);
        free(server);
        return NULL;
    }

    // 创建UDP套接字
    server->udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server->udp_sockfd < 0) {
        perror("socket failed");
        close(server->tcp_sockfd);
        queue_destroy(server->queue);
        free(server);
        return NULL;
    }

    // 初始化地址
    memset(&server->tcp_addr, 0, sizeof(server->tcp_addr));
    server->tcp_addr.sin_family = AF_INET;
    server->tcp_addr.sin_addr.s_addr = INADDR_ANY;
    server->tcp_addr.sin_port = htons(server->config.client1_port);

    memset(&server->udp_addr, 0, sizeof(server->udp_addr));
    server->udp_addr.sin_family = AF_INET;
    server->udp_addr.sin_addr.s_addr = INADDR_ANY;
    server->udp_addr.sin_port = htons(server->config.client2_port);

    // 绑定TCP端口
    if (bind(server->tcp_sockfd, (struct sockaddr*)&server->tcp_addr, sizeof(server->tcp_addr)) < 0) {
        perror("bind failed");
        close(server->tcp_sockfd);
        close(server->udp_sockfd);
        queue_destroy(server->queue);
        free(server);
        return NULL;
    }

    // 绑定UDP端口
    if (bind(server->udp_sockfd, (struct sockaddr*)&server->udp_addr, sizeof(server->udp_addr)) < 0) {
        perror("bind failed");
        close(server->tcp_sockfd);
        close(server->udp_sockfd);
        queue_destroy(server->queue);
        free(server);
        return NULL;
    }

    // 监听TCP连接
    if (listen(server->tcp_sockfd, 10) < 0) {
        perror("listen failed");
        close(server->tcp_sockfd);
        close(server->udp_sockfd);
        queue_destroy(server->queue);
        free(server);
        return NULL;
    }

    // 初始化运行状态
    server->running = 1;
    memset(server->filter_levels, 1, sizeof(server->filter_levels)); // 默认不过滤
    pthread_mutex_init(&server->filter_mutex, NULL);

    // 分配消费者线程
    server->consumer_threads = (pthread_t*)malloc(sizeof(pthread_t) * server->config.consumer_threads);
    if (!server->consumer_threads) {
        perror("malloc failed");
        close(server->tcp_sockfd);
        close(server->udp_sockfd);
        queue_destroy(server->queue);
        pthread_mutex_destroy(&server->filter_mutex);
        free(server);
        return NULL;
    }

    return server;
}

/**
 * 启动服务端
 * @param server 服务端实例
 * @return 错误码
 */
int server_start(LogServer* server) {
    if (!server) {
        return ERROR_SOCKET;
    }

    // 创建TCP接收线程
    if (pthread_create(&server->tcp_thread, NULL, tcp_receive_thread, server) != 0) {
        perror("pthread_create failed");
        return ERROR_SOCKET;
    }

    // 创建UDP接收线程
    if (pthread_create(&server->udp_thread, NULL, udp_receive_thread, server) != 0) {
        perror("pthread_create failed");
        pthread_cancel(server->tcp_thread);
        return ERROR_SOCKET;
    }

    // 创建消费者线程
    for (int i = 0; i < server->config.consumer_threads; i++) {
        if (pthread_create(&server->consumer_threads[i], NULL, consumer_thread, server) != 0) {
            perror("pthread_create failed");
            // 取消已创建的线程
            pthread_cancel(server->tcp_thread);
            pthread_cancel(server->udp_thread);
            for (int j = 0; j < i; j++) {
                pthread_cancel(server->consumer_threads[j]);
            }
            return ERROR_SOCKET;
        }
    }

    return ERROR_SUCCESS;
}

/**
 * 停止服务端
 * @param server 服务端实例
 */
void server_stop(LogServer* server) {
    if (!server) {
        return;
    }

    server->running = 0;

    // 取消线程
    pthread_cancel(server->tcp_thread);
    pthread_cancel(server->udp_thread);
    for (int i = 0; i < server->config.consumer_threads; i++) {
        pthread_cancel(server->consumer_threads[i]);
    }

    // 等待线程结束
    pthread_join(server->tcp_thread, NULL);
    pthread_join(server->udp_thread, NULL);
    for (int i = 0; i < server->config.consumer_threads; i++) {
        pthread_join(server->consumer_threads[i], NULL);
    }
}

/**
 * 清理服务端
 * @param server 服务端实例
 */
void server_cleanup(LogServer* server) {
    if (server) {
        close(server->tcp_sockfd);
        close(server->udp_sockfd);
        queue_destroy(server->queue);
        pthread_mutex_destroy(&server->filter_mutex);
        free(server->consumer_threads);
        free(server);
    }
}

/**
 * 解析配置文件
 * @param config_file 配置文件路径
 * @param config 配置结构体
 * @return 错误码
 */
int parse_server_config(const char* config_file, ServerConfig* config) {
    if (!config_file || !config) {
        return ERROR_CONFIG;
    }

    // 默认配置
    config->client1_port = 8888;
    config->client2_port = 8889;
    config->queue_capacity = MAX_QUEUE_SIZE;
    config->consumer_threads = 2;

    FILE* fp = fopen(config_file, "r");
    if (!fp) {
        // 配置文件不存在，使用默认配置
        return ERROR_SUCCESS;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // 去除换行符
        line[strcspn(line, "\n")] = '\0';
        
        // 解析配置项
        char key[64], value[64];
        if (sscanf(line, "%[^=]=%s", key, value) == 2) {
            if (strcmp(key, "client1_port") == 0) {
                config->client1_port = atoi(value);
            } else if (strcmp(key, "client2_port") == 0) {
                config->client2_port = atoi(value);
            } else if (strcmp(key, "queue_capacity") == 0) {
                config->queue_capacity = atoi(value);
            } else if (strcmp(key, "consumer_threads") == 0) {
                config->consumer_threads = atoi(value);
            }
        }
    }

    fclose(fp);
    return ERROR_SUCCESS;
}

/**
 * 设置日志过滤
 * @param server 服务端实例
 * @param levels 过滤级别数组
 * @param count 级别数量
 */
void set_log_filter(LogServer* server, int* levels, int count) {
    if (!server) {
        return;
    }

    pthread_mutex_lock(&server->filter_mutex);
    memset(server->filter_levels, 0, sizeof(server->filter_levels));
    for (int i = 0; i < count; i++) {
        if (levels[i] >= 0 && levels[i] < 4) {
            server->filter_levels[levels[i]] = 1;
        }
    }
    pthread_mutex_unlock(&server->filter_mutex);
}

/**
 * 取消日志过滤
 * @param server 服务端实例
 */
void clear_log_filter(LogServer* server) {
    if (!server) {
        return;
    }

    pthread_mutex_lock(&server->filter_mutex);
    memset(server->filter_levels, 1, sizeof(server->filter_levels));
    pthread_mutex_unlock(&server->filter_mutex);
}

/**
 * 导出日志
 * @param start_time 开始时间
 * @param end_time 结束时间
 * @param level 日志级别
 * @return 错误码
 */
int export_logs(time_t start_time, time_t end_time, log_level level) {
    DBConnection* db = db_init();
    if (!db) {
        return ERROR_DATABASE;
    }

    // 生成文件名
    char file_path[256];
    struct tm* tm_info = localtime(&start_time);
    char level_str[10];
    strcpy(level_str, log_level_str[level]);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y%m%d", tm_info);
    snprintf(file_path, sizeof(file_path), "/log_export/%s_%s.log", level_str, time_str);

    // 创建目录
    system("mkdir -p /log_export");

    int ret = db_export_logs(db, level, start_time, end_time, file_path);
    db_close(db);
    return ret;
}