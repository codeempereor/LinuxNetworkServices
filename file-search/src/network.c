/*****************************************************************
 > File Name:    network.c
 > Author:       三道渊
 > Description:  网络通信模块，实现TCP服务端和客户端通信
 > Created Time: 2026年03月03日 星期六 13时01分11秒
 *****************************************************************/

#include "../include/common.h"

/**
 * @brief 初始化TCP服务端
 * @return 成功返回监听socket，失败返回-1
 */
int server_init(void)
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        write_log("socket creation failed");
        return -1;
    }
    
    /* 设置地址重用 */
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        write_log("setsockopt failed");
        close(server_sock);
        return -1;
    }
    
    /* 绑定地址和端口 */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        write_log("bind failed");
        close(server_sock);
        return -1;
    }
    
    /* 开始监听 */
    if (listen(server_sock, MAX_CLIENTS) < 0)
    {
        write_log("listen failed");
        close(server_sock);
        return -1;
    }
    
    write_log("Server listening on port %d", SERVER_PORT);
    return server_sock;
}

/**
 * @brief 解析检索命令
 * @param cmd 命令字符串
 * @param cond 检索条件结构
 * @return 成功返回0，失败返回-1
 */
int parse_search_command(const char *cmd, SearchCondition *cond)
{
    memset(cond, 0, sizeof(SearchCondition));
    
    /* 检查是否是SEARCH命令 */
    if (strncmp(cmd, "SEARCH ", 7) != 0)
    {
        return -1;
    }
    
    const char *p = cmd + 7;
    char token[MAX_BUFFER_SIZE];
    
    while (*p != '\0' && *p != '\n')
    {
        /* 跳过空格 */
        while (*p == ' ') p++;
        if (*p == '\0' || *p == '\n') break;
        
        /* 读取一个条件 */
        int i = 0;
        while (*p != ' ' && *p != '\0' && *p != '\n' && i < (int)sizeof(token) - 1)
        {
            token[i++] = *p++;
        }
        token[i] = '\0';
        
        /* 解析条件 */
        if (strncmp(token, "filename=", 9) == 0)
        {
            cond->has_filename = 1;
            strncpy(cond->filename, token + 9, MAX_FILENAME_LENGTH - 1);
        }
        else if (strncmp(token, "filename_like=", 14) == 0)
        {
            cond->has_filename_like = 1;
            strncpy(cond->filename_like, token + 14, MAX_FILENAME_LENGTH - 1);
        }
        else if (strncmp(token, "size>", 5) == 0)
        {
            cond->has_size_gt = 1;
            cond->size_gt = atol(token + 5);
        }
        else if (strncmp(token, "mtime>", 6) == 0)
        {
            cond->has_mtime_gt = 1;
            cond->mtime_gt = atol(token + 6);
        }
        else if (strncmp(token, "regex=", 6) == 0)
        {
            cond->has_regex = 1;
            strncpy(cond->regex_pattern, token + 6, MAX_FILENAME_LENGTH - 1);
        }
    }
    
    return 0;
}

/**
 * @brief 检查路径是否合法
 * @param path 路径字符串
 * @return 合法返回1，非法返回0
 */
int check_path_valid(const char *path)
{
    const char *forbidden = FORBIDDEN_PATHS;
    char forbidden_copy[MAX_PATH_LENGTH];
    strncpy(forbidden_copy, forbidden, sizeof(forbidden_copy) - 1);
    forbidden_copy[sizeof(forbidden_copy) - 1] = '\0';
    
    char *token = strtok(forbidden_copy, ",");
    while (token != NULL)
    {
        if (strstr(path, token) != NULL)
        {
            return 0;
        }
        token = strtok(NULL, ",");
    }
    
    return 1;
}

/**
 * @brief 执行文件检索
 * @param cond 检索条件
 * @param result 结果结构
 */
void search_files(SearchCondition *cond, SearchResult *result)
{
    /* 初始化结果结构 */
    result->capacity = 100;
    result->count = 0;
    result->files = malloc(result->capacity * sizeof(FileInfo));
    if (result->files == NULL)
    {
        return;
    }
    
    /* 先在内存索引中搜索 */
    pthread_mutex_lock(&g_index_mutex);
    if (g_file_index_root != NULL)
    {
        traverse_tree(g_file_index_root, result, cond);
    }
    pthread_mutex_unlock(&g_index_mutex);
    
    /* 如果内存索引中没有找到，查询数据库 */
    if (result->count == 0)
    {
        db_search_files(cond, result);
    }
}

/**
 * @brief 释放检索结果内存
 * @param result 结果结构
 */
void free_search_result(SearchResult *result)
{
    if (result->files != NULL)
    {
        free(result->files);
        result->files = NULL;
    }
    result->count = 0;
    result->capacity = 0;
}

/**
 * @brief 客户端处理线程
 * @param arg 客户端socket
 * @return NULL
 */
void* client_handler(void *arg)
{
    int client_sock = *(int*)arg;
    free(arg);
    
    char buffer[MAX_BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    /* 获取客户端IP */
    getpeername(client_sock, (struct sockaddr*)&client_addr, &addr_len);
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    
    write_log("Client connected: %s", client_ip);
    
    while (1)
    {
        /* 接收命令 */
        memset(buffer, 0, sizeof(buffer));
        int recv_len = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (recv_len <= 0)
        {
            break;
        }
        
        buffer[recv_len] = '\0';
        write_log("Received from %s: %s", client_ip, buffer);
        
        /* 处理STAT命令 */
        if (strncmp(buffer, "STAT ", 5) == 0)
        {
            char filename[MAX_FILENAME_LENGTH];
            sscanf(buffer + 5, "%s", filename);
            int count = db_get_search_count(filename);
            
            char response[MAX_BUFFER_SIZE];
            snprintf(response, sizeof(response), "File '%s' searched %d times\n", filename, count);
            send(client_sock, response, strlen(response), 0);
            continue;
        }
        
        /* 解析SEARCH命令 */
        SearchCondition cond;
        if (parse_search_command(buffer, &cond) != 0)
        {
            send(client_sock, "INVALID COMMAND\n", 16, 0);
            continue;
        }
        
        /* 执行检索 */
        SearchResult result;
        search_files(&cond, &result);
        
        /* 记录检索日志 */
        db_insert_search_record(client_ip, buffer, result.count);
        
        /* 发送结果 */
        if (result.count == 0)
        {
            send(client_sock, "NO RESULTS\n", 11, 0);
        }
        else
        {
            /* 发送表头 */
            char header[] = "+----------------+----------------------+--------+----------+---------------------+\n";
            send(client_sock, header, strlen(header), 0);
            
            char columns[] = "| filename       | path                 | size   | mtime    |                     |\n";
            send(client_sock, columns, strlen(columns), 0);
            send(client_sock, header, strlen(header), 0);
            
            /* 发送数据行 */
            for (int i = 0; i < result.count; i++)
            {
                char size_str[16];
                char time_str[32];
                format_size(result.files[i].size, size_str, sizeof(size_str));
                format_time(result.files[i].mtime, time_str, sizeof(time_str));
                
                char row[MAX_BUFFER_SIZE];
                snprintf(row, sizeof(row), "| %-14s | %-20s | %-6s | %s |\n",
                        result.files[i].filename,
                        result.files[i].path,
                        size_str,
                        time_str);
                send(client_sock, row, strlen(row), 0);
            }
            
            send(client_sock, header, strlen(header), 0);
            
            /* 发送统计信息 */
            char stats[64];
            snprintf(stats, sizeof(stats), "Total: %d files\n", result.count);
            send(client_sock, stats, strlen(stats), 0);
        }
        
        free_search_result(&result);
    }
    
    write_log("Client disconnected: %s", client_ip);
    close(client_sock);
    return NULL;
}
