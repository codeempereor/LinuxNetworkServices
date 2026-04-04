/*****************************************************************
 > File Name:    common.h
 > Author:       三道渊
 > Description:  公共头文件，定义常量、结构体和函数声明
 > Created Time: 2026年03月03日 星期六 13时01分11秒
 *****************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <regex.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <mysql/mysql.h>

/**
 * 常量定义
 */
/* 服务器端口号 */
#define SERVER_PORT 7777
/* 最大缓冲区大小（字节） */
#define MAX_BUFFER_SIZE 4096
/* 最大路径长度 */
#define MAX_PATH_LENGTH 1024
/* 最大文件名长度 */
#define MAX_FILENAME_LENGTH 256
/* 最大并发客户端数 */
#define MAX_CLIENTS 20
/* 索引更新间隔（秒）- 5分钟 */
#define INDEX_UPDATE_INTERVAL 300
/* 记录保留天数 */
#define RECORD_RETENTION_DAYS 30
/* 日志文件路径 */
#define LOG_FILE "/var/log/file_search.log"
/* 搜索基础目录 */
#define SEARCH_BASE_DIR "/home/st/test_files/"
/* 禁止访问的路径 */
#define FORBIDDEN_PATHS "/root/,/proc/,/sys/,/dev/"

/**
 * 数据库配置
 */
/* 数据库主机地址 */
#define DB_HOST "localhost"
/* 数据库用户名 */
#define DB_USER "filesearch"
/* 数据库密码 */
#define DB_PASS "filesearch123"
/* 数据库名称 */
#define DB_NAME "file_search_db"

/**
 * 二叉树节点结构
 * 用于在内存中存储文件索引，按文件名进行二叉搜索树排序
 */
typedef struct TreeNode
{
    char filename[MAX_FILENAME_LENGTH];  /* 文件名 */
    char path[MAX_PATH_LENGTH];          /* 文件路径 */
    off_t size;                          /* 文件大小（字节） */
    time_t mtime;                        /* 最后修改时间 */
    struct TreeNode *left;               /* 左子节点（文件名小于当前节点） */
    struct TreeNode *right;              /* 右子节点（文件名大于当前节点） */
} TreeNode;

/**
 * 文件信息结构
 * 用于存储文件的基本信息
 */
typedef struct FileInfo
{
    char filename[MAX_FILENAME_LENGTH];  /* 文件名 */
    char path[MAX_PATH_LENGTH];          /* 文件路径 */
    off_t size;                          /* 文件大小（字节） */
    time_t mtime;                        /* 最后修改时间 */
} FileInfo;

/**
 * 检索条件结构
 * 用于存储用户的搜索条件
 */
typedef struct SearchCondition
{
    int has_filename;                    /* 是否指定了精确文件名 */
    char filename[MAX_FILENAME_LENGTH];  /* 精确文件名 */
    int has_filename_like;               /* 是否指定了模糊匹配 */
    char filename_like[MAX_FILENAME_LENGTH];  /* 模糊匹配模式 */
    int has_size_gt;                     /* 是否指定了大小条件 */
    off_t size_gt;                       /* 大小阈值（大于） */
    int has_mtime_gt;                    /* 是否指定了修改时间条件 */
    time_t mtime_gt;                     /* 时间阈值（大于） */
    int has_regex;                       /* 是否指定了正则表达式 */
    char regex_pattern[MAX_FILENAME_LENGTH];  /* 正则表达式模式 */
} SearchCondition;

/**
 * 检索结果结构
 * 用于存储搜索结果
 */
typedef struct SearchResult
{
    FileInfo *files;                     /* 文件信息数组 */
    int count;                           /* 结果数量 */
    int capacity;                        /* 数组容量 */
} SearchResult;

/**
 * 全局变量声明
 */
extern TreeNode *g_file_index_root;     /* 文件索引二叉树根节点 */
extern pthread_mutex_t g_index_mutex;   /* 索引互斥锁 */
extern MYSQL *g_db_conn;                /* 数据库连接 */

/**
 * 函数声明 - 文件索引模块
 */
/**
 * @brief 创建二叉树节点
 * @param filename 文件名
 * @param path 文件路径
 * @param size 文件大小
 * @param mtime 修改时间
 * @return 新创建的节点指针
 */
TreeNode* create_tree_node(const char *filename, const char *path, off_t size, time_t mtime);

/**
 * @brief 插入二叉树节点
 * @param root 根节点
 * @param new_node 新节点
 * @return 新的根节点
 */
TreeNode* insert_tree_node(TreeNode *root, TreeNode *new_node);

/**
 * @brief 释放二叉树内存
 * @param root 根节点
 */
void free_tree(TreeNode *root);

/**
 * @brief 搜索二叉树
 * @param root 根节点
 * @param filename 文件名
 * @return 找到的节点指针
 */
TreeNode* search_tree(TreeNode *root, const char *filename);

/**
 * @brief 遍历二叉树，收集符合条件的文件
 * @param root 根节点
 * @param result 结果存储
 * @param cond 搜索条件
 */
void traverse_tree(TreeNode *root, SearchResult *result, SearchCondition *cond);

/**
 * @brief 构建文件索引
 * @param base_dir 基础目录
 * @return 索引的文件数量
 */
int build_file_index(const char *base_dir);

/**
 * @brief 索引更新线程
 * @param arg 线程参数
 * @return NULL
 */
void* index_update_thread(void *arg);

/**
 * 函数声明 - 数据库模块
 */
/**
 * @brief 初始化数据库连接
 * @return 成功返回0，失败返回-1
 */
int db_init(void);

/**
 * @brief 关闭数据库连接
 */
void db_close(void);

/**
 * @brief 插入文件索引到数据库
 * @param root 二叉树根节点
 * @return 成功返回0，失败返回-1
 */
int db_insert_file_index(TreeNode *root);

/**
 * @brief 插入搜索记录到数据库
 * @param client_ip 客户端IP
 * @param search_cmd 搜索命令
 * @param match_count 匹配数量
 * @return 成功返回0，失败返回-1
 */
int db_insert_search_record(const char *client_ip, const char *search_cmd, int match_count);

/**
 * @brief 从数据库搜索文件
 * @param cond 搜索条件
 * @param result 结果存储
 * @return 成功返回0，失败返回-1
 */
int db_search_files(SearchCondition *cond, SearchResult *result);

/**
 * @brief 获取文件搜索次数
 * @param filename 文件名
 * @return 搜索次数
 */
int db_get_search_count(const char *filename);

/**
 * @brief 清理旧记录
 * @return 成功返回0，失败返回-1
 */
int db_clean_old_records(void);

/**
 * 函数声明 - 检索处理模块
 */
/**
 * @brief 解析搜索命令
 * @param cmd 命令字符串
 * @param cond 条件存储
 * @return 成功返回0，失败返回-1
 */
int parse_search_command(const char *cmd, SearchCondition *cond);

/**
 * @brief 检查路径是否合法
 * @param path 路径
 * @return 合法返回1，否则返回0
 */
int check_path_valid(const char *path);

/**
 * @brief 搜索文件
 * @param cond 搜索条件
 * @param result 结果存储
 */
void search_files(SearchCondition *cond, SearchResult *result);

/**
 * @brief 释放搜索结果
 * @param result 搜索结果
 */
void free_search_result(SearchResult *result);

/**
 * 函数声明 - 网络通信模块
 */
/**
 * @brief 初始化服务器
 * @return 成功返回监听socket，失败返回-1
 */
int server_init(void);

/**
 * @brief 客户端处理线程
 * @param arg 客户端socket
 * @return NULL
 */
void* client_handler(void *arg);

/**
 * @brief 发送搜索结果
 * @param sock 客户端socket
 * @param result 搜索结果
 * @return 成功返回0，失败返回-1
 */
int send_result(int sock, SearchResult *result);

/**
 * 函数声明 - 结果格式化模块
 */
/**
 * @brief 格式化文件大小
 * @param size_bytes 文件大小
 * @param output 输出缓冲区
 * @param output_size 缓冲区大小
 */
void format_size(off_t size_bytes, char *output, size_t output_size);

/**
 * @brief 格式化时间
 * @param mtime 时间戳
 * @param output 输出缓冲区
 * @param output_size 缓冲区大小
 */
void format_time(time_t mtime, char *output, size_t output_size);

/**
 * @brief 打印表格头部
 */
void print_table_header(void);

/**
 * @brief 打印表格行
 * @param file 文件信息
 */
void print_table_row(const FileInfo *file);

/**
 * @brief 打印表格尾部
 * @param total_count 总数量
 */
void print_table_footer(int total_count);

/**
 * 函数声明 - 工具函数
 */
/**
 * @brief 写入日志
 * @param format 格式字符串
 * @param ... 可变参数
 */
void write_log(const char *format, ...);

/**
 * @brief 创建守护进程
 * @return 成功返回0，失败返回-1
 */
int daemonize(void);

/**
 * @brief 信号处理函数
 * @param sig 信号
 */
void signal_handler(int sig);

#endif /* COMMON_H */
