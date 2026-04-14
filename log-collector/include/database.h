/*****************************************************************
 > File Name:    database.h
 > Author:       三道渊
 > Description:  数据库头文件，定义数据库操作函数
 > Created Time: 2026年04月14日 星期一 17时00分00秒
 *****************************************************************/

#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"

/**
 * 数据库连接结构体
 */
typedef struct {
    void* conn;            // 数据库连接
} DBConnection;

/**
 * 初始化数据库连接
 * @return 数据库连接实例
 */
DBConnection* db_init(void);

/**
 * 关闭数据库连接
 * @param db 数据库连接实例
 */
void db_close(DBConnection* db);

/**
 * 插入日志
 * @param db 数据库连接实例
 * @param log 日志条目
 * @return 错误码
 */
int db_insert_log(DBConnection* db, LogEntry* log);

/**
 * 按模块查询日志
 * @param db 数据库连接实例
 * @param level 日志级别
 * @param module 模块名
 * @param result 结果集
 * @return 错误码
 */
int db_query_by_module(DBConnection* db, log_level level, const char* module, FILE* result);

/**
 * 按时间范围查询日志
 * @param db 数据库连接实例
 * @param level 日志级别
 * @param start_time 开始时间
 * @param end_time 结束时间
 * @param result 结果集
 * @return 错误码
 */
int db_query_by_time(DBConnection* db, log_level level, time_t start_time, time_t end_time, FILE* result);

/**
 * 模糊查询日志
 * @param db 数据库连接实例
 * @param level 日志级别
 * @param pattern 模糊匹配模式
 * @param result 结果集
 * @return 错误码
 */
int db_query_by_pattern(DBConnection* db, log_level level, const char* pattern, FILE* result);

/**
 * 导出日志到文件
 * @param db 数据库连接实例
 * @param level 日志级别
 * @param start_time 开始时间
 * @param end_time 结束时间
 * @param file_path 文件路径
 * @return 错误码
 */
int db_export_logs(DBConnection* db, log_level level, time_t start_time, time_t end_time, const char* file_path);

#endif /* DATABASE_H */