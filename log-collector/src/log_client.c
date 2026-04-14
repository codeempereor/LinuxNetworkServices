/*****************************************************************
 > File Name:    log_client.c
 > Author:       三道渊
 > Description:  客户端实现文件
 > Created Time: 2026年04月14日 星期一 17时02分00秒
 *****************************************************************/

#include "client.h"

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

    // 初始化客户端
    LogClient* client = client_init(argv[1]);
    if (!client) {
        printf("Failed to initialize client\n");
        return 1;
    }

    // 发送测试日志
    log_send(client, DEBUG, "test_module", "Debug message");
    log_send(client, INFO, "test_module", "Info message");
    log_send(client, WARN, "test_module", "Warn message");
    log_send(client, ERROR, "test_module", "Error message");

    // 清理客户端
    client_cleanup(client);
    return 0;
}



/**
 * 初始化客户端
 * @param config_file 配置文件路径
 * @return 客户端实例
 */
LogClient* client_init(const char* config_file) {
    LogClient* client = (LogClient*)malloc(sizeof(LogClient));
    if (!client) {
        perror("malloc failed");
        return NULL;
    }

    // 初始化配置
    if (parse_client_config(config_file, &client->config) != ERROR_SUCCESS) {
        free(client);
        return NULL;
    }

    // 创建套接字
    if (client->config.protocol == PROTOCOL_TCP) {
        client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    } else {
        client->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    }

    if (client->sockfd < 0) {
        perror("socket failed");
        free(client);
        return NULL;
    }

    // 初始化服务器地址
    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(client->config.server_port);
    if (inet_pton(AF_INET, client->config.server_ip, &client->server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(client->sockfd);
        free(client);
        return NULL;
    }

    // 初始化缓存
    client->cache_size = 0;
    client->cache_head = 0;
    client->cache_tail = 0;
    pthread_mutex_init(&client->cache_mutex, NULL);

    // 如果是TCP，尝试连接
    if (client->config.protocol == PROTOCOL_TCP) {
        if (connect(client->sockfd, (struct sockaddr*)&client->server_addr, sizeof(client->server_addr)) < 0) {
            // 连接失败，不影响初始化
            printf("Warning: Failed to connect to server, will cache logs\n");
        }
    }

    return client;
}

/**
 * 发送日志
 * @param client 客户端实例
 * @param level 日志级别
 * @param module 模块名
 * @param content 日志内容
 * @return 错误码
 */
int log_send(LogClient* client, log_level level, const char* module, const char* content) {
    if (!client || !module || !content) {
        return ERROR_SEND;
    }

    // 创建日志条目
    LogEntry log;
    log.timestamp = time(NULL);
    gethostname(log.ip, sizeof(log.ip));
    strncpy(log.module, module, sizeof(log.module) - 1);
    log.level = level;
    strncpy(log.content, content, sizeof(log.content) - 1);

    // 格式化日志
    char buffer[MAX_LOG_SIZE];
    snprintf(buffer, sizeof(buffer), "[%ld][%s][%s][%s]%s",
             log.timestamp, log.ip, log.module, log_level_str[log.level], log.content);

    int sent = 0;
    if (client->config.protocol == PROTOCOL_TCP) {
        // TCP发送
        sent = send(client->sockfd, buffer, strlen(buffer), 0);
    } else {
        // UDP发送
        sent = sendto(client->sockfd, buffer, strlen(buffer), 0,
                     (struct sockaddr*)&client->server_addr, sizeof(client->server_addr));
    }

    if (sent < 0) {
        // 发送失败，缓存日志
        pthread_mutex_lock(&client->cache_mutex);
        if (client->cache_size < MAX_CACHE_SIZE) {
            client->cache[client->cache_tail] = log;
            client->cache_tail = (client->cache_tail + 1) % MAX_CACHE_SIZE;
            client->cache_size++;
        } else {
            // 缓存满，覆盖最早的日志
            client->cache[client->cache_head] = log;
            client->cache_head = (client->cache_head + 1) % MAX_CACHE_SIZE;
        }
        pthread_mutex_unlock(&client->cache_mutex);
        return ERROR_SEND;
    }

    // 发送成功，尝试重发缓存的日志
    resend_cache(client);
    return ERROR_SUCCESS;
}

/**
 * 重发缓存的日志
 * @param client 客户端实例
 * @return 成功重发的日志数量
 */
int resend_cache(LogClient* client) {
    if (!client) {
        return 0;
    }

    int count = 0;
    pthread_mutex_lock(&client->cache_mutex);

    while (client->cache_size > 0) {
        LogEntry log = client->cache[client->cache_head];
        
        // 格式化日志
        char buffer[MAX_LOG_SIZE];
        snprintf(buffer, sizeof(buffer), "[%ld][%s][%s][%s]%s",
                 log.timestamp, log.ip, log.module, log_level_str[log.level], log.content);

        int sent = 0;
        if (client->config.protocol == PROTOCOL_TCP) {
            sent = send(client->sockfd, buffer, strlen(buffer), 0);
        } else {
            sent = sendto(client->sockfd, buffer, strlen(buffer), 0,
                         (struct sockaddr*)&client->server_addr, sizeof(client->server_addr));
        }

        if (sent < 0) {
            break;
        }

        // 移除已发送的日志
        client->cache_head = (client->cache_head + 1) % MAX_CACHE_SIZE;
        client->cache_size--;
        count++;
    }

    pthread_mutex_unlock(&client->cache_mutex);
    return count;
}

/**
 * 清理客户端
 * @param client 客户端实例
 */
void client_cleanup(LogClient* client) {
    if (client) {
        close(client->sockfd);
        pthread_mutex_destroy(&client->cache_mutex);
        free(client);
    }
}

/**
 * 解析配置文件
 * @param config_file 配置文件路径
 * @param config 配置结构体
 * @return 错误码
 */
int parse_client_config(const char* config_file, ClientConfig* config) {
    if (!config_file || !config) {
        return ERROR_CONFIG;
    }

    // 默认配置
    config->protocol = PROTOCOL_TCP;
    strcpy(config->server_ip, "127.0.0.1");
    config->server_port = SERVER_PORT;
    config->max_cache = MAX_CACHE_SIZE;

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
            if (strcmp(key, "protocol") == 0) {
                if (strcmp(value, "UDP") == 0) {
                    config->protocol = PROTOCOL_UDP;
                } else {
                    config->protocol = PROTOCOL_TCP;
                }
            } else if (strcmp(key, "server_ip") == 0) {
                strncpy(config->server_ip, value, sizeof(config->server_ip) - 1);
            } else if (strcmp(key, "server_port") == 0) {
                config->server_port = atoi(value);
            }
        }
    }

    fclose(fp);
    return ERROR_SUCCESS;
}