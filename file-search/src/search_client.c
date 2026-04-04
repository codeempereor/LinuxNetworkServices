/*****************************************************************
 > File Name:    search_client.c
 > Author:       三道渊
 > Description:  检索客户端程序，实现命令行交互和远程检索
 > Created Time: 2026年03月03日 星期六 13时01分11秒
 *****************************************************************/

#include "../include/common.h"

/**
 * @brief 打印使用帮助
 */
void print_help(void)
{
    printf("Linux File Search Client\n");
    printf("========================\n\n");
    printf("Commands:\n");
    printf("  SEARCH filename=<name>           - Exact filename match\n");
    printf("  SEARCH filename_like=<pattern>   - Fuzzy filename match\n");
    printf("  SEARCH size><bytes>               - File size greater than\n");
    printf("  SEARCH mtime><timestamp>          - Modified time greater than\n");
    printf("  SEARCH regex=<pattern>           - Regular expression match\n");
    printf("  STAT <filename>                  - Get search statistics\n");
    printf("  help                             - Show this help\n");
    printf("  quit                             - Exit client\n\n");
    printf("Examples:\n");
    printf("  SEARCH filename=test.txt\n");
    printf("  SEARCH filename_like=%%test%%\n");
    printf("  SEARCH filename_like=%%.c%% size>1024\n");
    printf("  SEARCH regex=^test.*.txt$\n");
    printf("  STAT test.txt\n");
}

/**
 * @brief 主函数
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 程序退出状态码
 */
int main(int argc, char *argv[])
{
    char *server_ip = "127.0.0.1";
    int server_port = SERVER_PORT;
    
    /* 解析命令行参数 */
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 && i + 1 < argc)
        {
            server_ip = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
        {
            server_port = atoi(argv[i + 1]);
            i++;
        }
    }
    
    /* 创建socket */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket creation failed");
        return 1;
    }
    
    /* 连接服务端 */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        close(sock);
        return 1;
    }
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }
    
    printf("Connected to server %s:%d\n", server_ip, server_port);
    print_help();
    printf("\n");
    
    /* 命令行交互 */
    char input[MAX_BUFFER_SIZE];
    char response[MAX_BUFFER_SIZE * 10];
    
    while (1)
    {
        printf("> ");
        fflush(stdout);
        
        /* 读取用户输入 */
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }
        
        /* 去除换行符 */
        input[strcspn(input, "\n")] = '\0';
        
        /* 处理quit命令 */
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0)
        {
            break;
        }
        
        /* 处理help命令 */
        if (strcmp(input, "help") == 0)
        {
            print_help();
            continue;
        }
        
        /* 发送命令到服务端 */
        strcat(input, "\n");
        if (send(sock, input, strlen(input), 0) < 0)
        {
            perror("send failed");
            break;
        }
        
        /* 接收响应 */
        memset(response, 0, sizeof(response));
        int total_recv = 0;
        int recv_len;
        
        while ((recv_len = recv(sock, response + total_recv, 
                               sizeof(response) - total_recv - 1, 0)) > 0)
        {
            total_recv += recv_len;
            response[total_recv] = '\0';
            
            /* 检查是否接收完整 */
            if (strstr(response, "\n") != NULL)
            {
                break;
            }
        }
        
        if (recv_len < 0)
        {
            perror("recv failed");
            break;
        }
        
        /* 显示响应 */
        printf("%s", response);
    }
    
    printf("Disconnected from server\n");
    close(sock);
    return 0;
}
