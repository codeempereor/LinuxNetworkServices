/*****************************************************************
 > File Name:    database.c
 > Author:       三道渊
 > Description:  数据库操作模块，实现MySQL连接和数据操作
 > Created Time: 2026年03月03日 星期六 13时01分11秒
 *****************************************************************/

#include "../include/common.h"

/* 全局变量定义 */
MYSQL *g_db_conn = NULL;

/**
 * @brief 初始化数据库连接
 * @return 成功返回0，失败返回-1
 */
int db_init(void)
{
    g_db_conn = mysql_init(NULL);
    if (g_db_conn == NULL)
    {
        write_log("mysql_init failed");
        return -1;
    }
    
    /* 连接数据库 */
    if (mysql_real_connect(g_db_conn, DB_HOST, DB_USER, DB_PASS, 
                          DB_NAME, 0, NULL, 0) == NULL)
    {
        write_log("mysql_real_connect failed: %s", mysql_error(g_db_conn));
        mysql_close(g_db_conn);
        g_db_conn = NULL;
        return -1;
    }
    
    write_log("Database connected successfully");
    return 0;
}

/**
 * @brief 关闭数据库连接
 */
void db_close(void)
{
    if (g_db_conn != NULL)
    {
        mysql_close(g_db_conn);
        g_db_conn = NULL;
        write_log("Database connection closed");
    }
}

/**
 * @brief 递归插入二叉树节点到数据库
 * @param root 根节点
 * @param stmt MySQL预处理语句
 * @param count 插入计数指针
 */
static void insert_tree_to_db(TreeNode *root, MYSQL_STMT *stmt, int *count)
{
    if (root == NULL)
    {
        return;
    }
    
    /* 中序遍历插入 */
    insert_tree_to_db(root->left, stmt, count);
    
    /* 准备参数 */
    MYSQL_BIND bind[4];
    memset(bind, 0, sizeof(bind));
    
    /* filename */
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = root->filename;
    bind[0].buffer_length = strlen(root->filename);
    
    /* path */
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = root->path;
    bind[1].buffer_length = strlen(root->path);
    
    /* size */
    bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[2].buffer = &root->size;
    bind[2].is_unsigned = 0;
    
    /* mtime */
    bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[3].buffer = &root->mtime;
    bind[3].is_unsigned = 0;
    
    /* 绑定参数 */
    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        write_log("mysql_stmt_bind_param failed: %s", mysql_stmt_error(stmt));
        return;
    }
    
    /* 执行插入 */
    if (mysql_stmt_execute(stmt) != 0)
    {
        write_log("mysql_stmt_execute failed: %s", mysql_stmt_error(stmt));
    }
    else
    {
        (*count)++;
    }
    
    insert_tree_to_db(root->right, stmt, count);
}

/**
 * @brief 将文件索引插入数据库
 * @param root 二叉树根节点
 * @return 插入的记录数
 */
int db_insert_file_index(TreeNode *root)
{
    if (g_db_conn == NULL || root == NULL)
    {
        return 0;
    }
    
    /* 清空旧数据 */
    if (mysql_query(g_db_conn, "TRUNCATE TABLE file_index") != 0)
    {
        write_log("TRUNCATE TABLE failed: %s", mysql_error(g_db_conn));
        return 0;
    }
    
    /* 准备预处理语句 */
    const char *sql = "INSERT INTO file_index (filename, path, size, mtime) VALUES (?, ?, ?, ?)";
    MYSQL_STMT *stmt = mysql_stmt_init(g_db_conn);
    if (stmt == NULL)
    {
        write_log("mysql_stmt_init failed");
        return 0;
    }
    
    if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0)
    {
        write_log("mysql_stmt_prepare failed: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }
    
    int count = 0;
    insert_tree_to_db(root, stmt, &count);
    
    mysql_stmt_close(stmt);
    write_log("Inserted %d records to file_index", count);
    return count;
}

/**
 * @brief 插入检索记录
 * @param client_ip 客户端IP
 * @param search_cmd 检索命令
 * @param match_count 匹配结果数
 * @return 成功返回0，失败返回-1
 */
int db_insert_search_record(const char *client_ip, const char *search_cmd, int match_count)
{
    if (g_db_conn == NULL)
    {
        return -1;
    }
    
    const char *sql = "INSERT INTO search_record (client_ip, search_cmd, match_count) VALUES (?, ?, ?)";
    MYSQL_STMT *stmt = mysql_stmt_init(g_db_conn);
    if (stmt == NULL)
    {
        return -1;
    }
    
    if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0)
    {
        mysql_stmt_close(stmt);
        return -1;
    }
    
    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));
    
    /* client_ip */
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)client_ip;
    bind[0].buffer_length = strlen(client_ip);
    
    /* search_cmd */
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)search_cmd;
    bind[1].buffer_length = strlen(search_cmd);
    
    /* match_count */
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = &match_count;
    bind[2].is_unsigned = 0;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        mysql_stmt_close(stmt);
        return -1;
    }
    
    int ret = mysql_stmt_execute(stmt);
    mysql_stmt_close(stmt);
    
    return (ret == 0) ? 0 : -1;
}

/**
 * @brief 从数据库搜索文件
 * @param cond 检索条件
 * @param result 结果结构
 * @return 匹配的记录数
 */
int db_search_files(SearchCondition *cond, SearchResult *result)
{
    if (g_db_conn == NULL)
    {
        return 0;
    }
    
    char sql[MAX_BUFFER_SIZE];
    strcpy(sql, "SELECT filename, path, size, mtime FROM file_index WHERE 1=1");
    
    /* 构建WHERE子句 */
    if (cond->has_filename)
    {
        strcat(sql, " AND filename='");
        strcat(sql, cond->filename);
        strcat(sql, "'");
    }
    
    if (cond->has_filename_like)
    {
        strcat(sql, " AND filename LIKE '%");
        strcat(sql, cond->filename_like);
        strcat(sql, "%'");
    }
    
    if (cond->has_size_gt)
    {
        char size_str[64];
        snprintf(size_str, sizeof(size_str), " AND size>%ld", cond->size_gt);
        strcat(sql, size_str);
    }
    
    if (cond->has_mtime_gt)
    {
        char mtime_str[64];
        snprintf(mtime_str, sizeof(mtime_str), " AND mtime>%ld", cond->mtime_gt);
        strcat(sql, mtime_str);
    }
    
    if (mysql_query(g_db_conn, sql) != 0)
    {
        write_log("db_search_files query failed: %s", mysql_error(g_db_conn));
        return 0;
    }
    
    MYSQL_RES *res = mysql_store_result(g_db_conn);
    if (res == NULL)
    {
        return 0;
    }
    
    MYSQL_ROW row;
    int count = 0;
    
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        if (result->count >= result->capacity)
        {
            int new_capacity = result->capacity * 2;
            FileInfo *new_files = realloc(result->files, new_capacity * sizeof(FileInfo));
            if (new_files == NULL)
            {
                break;
            }
            result->files = new_files;
            result->capacity = new_capacity;
        }
        
        strncpy(result->files[result->count].filename, row[0], MAX_FILENAME_LENGTH - 1);
        strncpy(result->files[result->count].path, row[1], MAX_PATH_LENGTH - 1);
        result->files[result->count].size = atol(row[2]);
        result->files[result->count].mtime = atol(row[3]);
        result->count++;
        count++;
    }
    
    mysql_free_result(res);
    return count;
}

/**
 * @brief 获取文件的检索次数
 * @param filename 文件名
 * @return 检索次数
 */
int db_get_search_count(const char *filename)
{
    if (g_db_conn == NULL)
    {
        return 0;
    }
    
    char sql[MAX_BUFFER_SIZE];
    snprintf(sql, sizeof(sql), 
             "SELECT COUNT(*) FROM search_record WHERE search_cmd LIKE '%%filename=%s%%'",
             filename);
    
    if (mysql_query(g_db_conn, sql) != 0)
    {
        return 0;
    }
    
    MYSQL_RES *res = mysql_store_result(g_db_conn);
    if (res == NULL)
    {
        return 0;
    }
    
    MYSQL_ROW row = mysql_fetch_row(res);
    int count = (row != NULL) ? atoi(row[0]) : 0;
    
    mysql_free_result(res);
    return count;
}

/**
 * @brief 清理过期检索记录
 * @return 删除的记录数
 */
int db_clean_old_records(void)
{
    if (g_db_conn == NULL)
    {
        return 0;
    }
    
    char sql[MAX_BUFFER_SIZE];
    time_t now = time(NULL);
    time_t expire_time = now - (RECORD_RETENTION_DAYS * 24 * 3600);
    
    snprintf(sql, sizeof(sql), 
             "DELETE FROM search_record WHERE search_time < FROM_UNIXTIME(%ld)",
             expire_time);
    
    if (mysql_query(g_db_conn, sql) != 0)
    {
        write_log("db_clean_old_records failed: %s", mysql_error(g_db_conn));
        return 0;
    }
    
    int deleted = mysql_affected_rows(g_db_conn);
    write_log("Cleaned %d old search records", deleted);
    return deleted;
}
