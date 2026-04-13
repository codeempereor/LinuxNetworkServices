/*****************************************************************
 > File Name:    database.h
 > Author:       三道渊
 > Description:  数据库操作相关头文件
 > Created Time: 2026年04月13日 星期日 19时30分00秒
 *****************************************************************/

#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"

/**
 * 函数声明
 */
/**
 * @brief 连接数据库
 * @return 数据库连接指针
 */
MYSQL *db_connect();

/**
 * @brief 断开数据库连接
 * @param mysql 数据库连接指针
 */
void db_disconnect(MYSQL *mysql);

/**
 * @brief 插入传输任务
 * @param mysql 数据库连接指针
 * @param task 传输任务
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int db_insert_task(MYSQL *mysql, TransferTask *task);

/**
 * @brief 更新传输任务
 * @param mysql 数据库连接指针
 * @param task 传输任务
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int db_update_task(MYSQL *mysql, TransferTask *task);

/**
 * @brief 获取传输任务
 * @param mysql 数据库连接指针
 * @param filename 文件名
 * @param task 传输任务
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int db_get_task(MYSQL *mysql, const char *filename, TransferTask *task);

/**
 * @brief 更新传输进度
 * @param mysql 数据库连接指针
 * @param filename 文件名
 * @param completed_blocks 已完成分块数
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int db_update_progress(MYSQL *mysql, const char *filename, int completed_blocks);

/**
 * @brief 更新传输状态
 * @param mysql 数据库连接指针
 * @param filename 文件名
 * @param status 状态
 * @param md5 MD5值
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int db_update_status(MYSQL *mysql, const char *filename, int status, const char *md5);

#endif // DATABASE_H
