/*****************************************************************
 > File Name:    database.c
 > Author:       三道渊
 > Description:  数据库实现文件
 > Created Time: 2026年04月14日 星期一 17时05分00秒
 *****************************************************************/

#include "database.h"
#include <mysql/mysql.h>

/**
 * 初始化数据库连接
 * @return 数据库连接实例
 */
DBConnection* db_init(void) {
    DBConnection* db = (DBConnection*)malloc(sizeof(DBConnection));
    if (!db) {
        perror("malloc failed");
        return NULL;
    }

    // 初始化MySQL连接
    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        perror("mysql_init failed");
        free(db);
        return NULL;
    }

    // 连接数据库
    conn = mysql_real_connect(conn, "localhost", "root", "1", "log_collector", 0, NULL, 0);
    if (!conn) {
        perror("mysql_real_connect failed");
        mysql_close(conn);
        free(db);
        return NULL;
    }

    db->conn = conn;
    return db;
}

/**
 * 关闭数据库连接
 * @param db 数据库连接实例
 */
void db_close(DBConnection* db) {
    if (db) {
        if (db->conn) {
            mysql_close((MYSQL*)db->conn);
        }
        free(db);
    }
}

/**
 * 插入日志
 * @param db 数据库连接实例
 * @param log 日志条目
 * @return 错误码
 */
int db_insert_log(DBConnection* db, LogEntry* log) {
    if (!db || !log) {
        return ERROR_DATABASE;
    }

    MYSQL* conn = (MYSQL*)db->conn;
    char query[1024];
    const char* table_name = NULL;

    // 根据日志级别选择表
    switch (log->level) {
        case DEBUG:
            table_name = "debug_log";
            break;
        case INFO:
            table_name = "info_log";
            break;
        case WARN:
            table_name = "warn_log";
            break;
        case ERROR:
            table_name = "error_log";
            break;
        default:
            return ERROR_DATABASE;
    }

    // 构造SQL语句
    snprintf(query, sizeof(query), "INSERT INTO %s (timestamp, client_ip, module, content) VALUES (%ld, '%s', '%s', '%s')",
             table_name, log->timestamp, log->ip, log->module, log->content);

    // 执行SQL
    if (mysql_query(conn, query) != 0) {
        printf("MySQL error: %s\n", mysql_error(conn));
        return ERROR_DATABASE;
    }

    return ERROR_SUCCESS;
}

/**
 * 按模块查询日志
 * @param db 数据库连接实例
 * @param level 日志级别
 * @param module 模块名
 * @param result 结果集
 * @return 错误码
 */
int db_query_by_module(DBConnection* db, log_level level, const char* module, FILE* result) {
    if (!db || !module || !result) {
        return ERROR_DATABASE;
    }

    MYSQL* conn = (MYSQL*)db->conn;
    char query[1024];
    const char* table_name = NULL;

    // 根据日志级别选择表
    switch (level) {
        case DEBUG:
            table_name = "debug_log";
            break;
        case INFO:
            table_name = "info_log";
            break;
        case WARN:
            table_name = "warn_log";
            break;
        case ERROR:
            table_name = "error_log";
            break;
        default:
            return ERROR_DATABASE;
    }

    // 构造SQL语句
    snprintf(query, sizeof(query), "SELECT * FROM %s WHERE module='%s'", table_name, module);

    // 执行SQL
    if (mysql_query(conn, query) != 0) {
        printf("MySQL error: %s\n", mysql_error(conn));
        return ERROR_DATABASE;
    }

    // 处理结果
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        printf("MySQL error: %s\n", mysql_error(conn));
        return ERROR_DATABASE;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL) {
        fprintf(result, "[%s][%s][%s][%s]%s\n", row[1], row[2], row[3], log_level_str[level], row[4]);
    }

    mysql_free_result(res);
    return ERROR_SUCCESS;
}

/**
 * 按时间范围查询日志
 * @param db 数据库连接实例
 * @param level 日志级别
 * @param start_time 开始时间
 * @param end_time 结束时间
 * @param result 结果集
 * @return 错误码
 */
int db_query_by_time(DBConnection* db, log_level level, time_t start_time, time_t end_time, FILE* result) {
    if (!db || !result) {
        return ERROR_DATABASE;
    }

    MYSQL* conn = (MYSQL*)db->conn;
    char query[1024];
    const char* table_name = NULL;

    // 根据日志级别选择表
    switch (level) {
        case DEBUG:
            table_name = "debug_log";
            break;
        case INFO:
            table_name = "info_log";
            break;
        case WARN:
            table_name = "warn_log";
            break;
        case ERROR:
            table_name = "error_log";
            break;
        default:
            return ERROR_DATABASE;
    }

    // 构造SQL语句
    snprintf(query, sizeof(query), "SELECT * FROM %s WHERE timestamp BETWEEN %ld AND %ld",
             table_name, start_time, end_time);

    // 执行SQL
    if (mysql_query(conn, query) != 0) {
        printf("MySQL error: %s\n", mysql_error(conn));
        return ERROR_DATABASE;
    }

    // 处理结果
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        printf("MySQL error: %s\n", mysql_error(conn));
        return ERROR_DATABASE;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL) {
        fprintf(result, "[%s][%s][%s][%s]%s\n", row[1], row[2], row[3], log_level_str[level], row[4]);
    }

    mysql_free_result(res);
    return ERROR_SUCCESS;
}

/**
 * 模糊查询日志
 * @param db 数据库连接实例
 * @param level 日志级别
 * @param pattern 模糊匹配模式
 * @param result 结果集
 * @return 错误码
 */
int db_query_by_pattern(DBConnection* db, log_level level, const char* pattern, FILE* result) {
    if (!db || !pattern || !result) {
        return ERROR_DATABASE;
    }

    MYSQL* conn = (MYSQL*)db->conn;
    char query[1024];
    const char* table_name = NULL;

    // 根据日志级别选择表
    switch (level) {
        case DEBUG:
            table_name = "debug_log";
            break;
        case INFO:
            table_name = "info_log";
            break;
        case WARN:
            table_name = "warn_log";
            break;
        case ERROR:
            table_name = "error_log";
            break;
        default:
            return ERROR_DATABASE;
    }

    // 构造SQL语句
    snprintf(query, sizeof(query), "SELECT * FROM %s WHERE content LIKE '%%%s%%'", table_name, pattern);

    // 执行SQL
    if (mysql_query(conn, query) != 0) {
        printf("MySQL error: %s\n", mysql_error(conn));
        return ERROR_DATABASE;
    }

    // 处理结果
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        printf("MySQL error: %s\n", mysql_error(conn));
        return ERROR_DATABASE;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL) {
        fprintf(result, "[%s][%s][%s][%s]%s\n", row[1], row[2], row[3], log_level_str[level], row[4]);
    }

    mysql_free_result(res);
    return ERROR_SUCCESS;
}

/**
 * 导出日志到文件
 * @param db 数据库连接实例
 * @param level 日志级别
 * @param start_time 开始时间
 * @param end_time 结束时间
 * @param file_path 文件路径
 * @return 错误码
 */
int db_export_logs(DBConnection* db, log_level level, time_t start_time, time_t end_time, const char* file_path) {
    if (!db || !file_path) {
        return ERROR_DATABASE;
    }

    FILE* fp = fopen(file_path, "w");
    if (!fp) {
        perror("fopen failed");
        return ERROR_DATABASE;
    }

    int ret = db_query_by_time(db, level, start_time, end_time, fp);
    fclose(fp);
    return ret;
}