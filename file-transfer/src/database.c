/*****************************************************************
 > File Name:    database.c
 > Author:       三道渊
 > Description:  数据库操作实现
 > Created Time: 2026年04月13日 星期日 19时30分00秒
 *****************************************************************/

#include "database.h"

/**
 * 数据库连接参数
 */
#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASSWORD ""
#define DB_NAME "transfer_db"

/**
 * @brief 连接数据库
 * @return 数据库连接指针
 */
MYSQL *db_connect() {
    MYSQL *mysql = mysql_init(NULL);
    if (mysql == NULL) {
        fprintf(stderr, "mysql_init failed\n");
        return NULL;
    }

    if (mysql_real_connect(mysql, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect failed: %s\n", mysql_error(mysql));
        mysql_close(mysql);
        return NULL;
    }

    return mysql;
}

/**
 * @brief 断开数据库连接
 * @param mysql 数据库连接指针
 */
void db_disconnect(MYSQL *mysql) {
    if (mysql != NULL) {
        mysql_close(mysql);
    }
}

/**
 * @brief 插入传输任务
 * @param mysql 数据库连接指针
 * @param task 传输任务
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int db_insert_task(MYSQL *mysql, TransferTask *task) {
    char query[1024];
    sprintf(query, "INSERT INTO transfer_task (filename, file_size, total_blocks, completed_blocks, client_ip, status) VALUES ('%s', %lld, %d, %d, '%s', %d)",
            task->filename, task->file_size, task->total_blocks, task->completed_blocks, task->client_ip, task->status);

    if (mysql_query(mysql, query) != 0) {
        fprintf(stderr, "mysql_query failed: %s\n", mysql_error(mysql));
        return ERROR_DATABASE;
    }

    task->id = mysql_insert_id(mysql);
    return ERROR_SUCCESS;
}

/**
 * @brief 更新传输任务
 * @param mysql 数据库连接指针
 * @param task 传输任务
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int db_update_task(MYSQL *mysql, TransferTask *task) {
    char query[1024];
    sprintf(query, "UPDATE transfer_task SET file_size = %lld, total_blocks = %d, completed_blocks = %d, client_ip = '%s', status = %d, md5 = '%s' WHERE id = %d",
            task->file_size, task->total_blocks, task->completed_blocks, task->client_ip, task->status, task->md5, task->id);

    if (mysql_query(mysql, query) != 0) {
        fprintf(stderr, "mysql_query failed: %s\n", mysql_error(mysql));
        return ERROR_DATABASE;
    }

    return ERROR_SUCCESS;
}

/**
 * @brief 获取传输任务
 * @param mysql 数据库连接指针
 * @param filename 文件名
 * @param task 传输任务
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int db_get_task(MYSQL *mysql, const char *filename, TransferTask *task) {
    char query[1024];
    sprintf(query, "SELECT id, file_size, total_blocks, completed_blocks, client_ip, status, md5 FROM transfer_task WHERE filename = '%s'", filename);

    if (mysql_query(mysql, query) != 0) {
        fprintf(stderr, "mysql_query failed: %s\n", mysql_error(mysql));
        return ERROR_DATABASE;
    }

    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result failed: %s\n", mysql_error(mysql));
        return ERROR_DATABASE;
    }

    int num_rows = mysql_num_rows(result);
    if (num_rows == 0) {
        mysql_free_result(result);
        return ERROR_SUCCESS;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    task->id = atoi(row[0]);
    task->file_size = atoll(row[1]);
    task->total_blocks = atoi(row[2]);
    task->completed_blocks = atoi(row[3]);
    strcpy(task->client_ip, row[4]);
    task->status = atoi(row[5]);
    if (row[6] != NULL) {
        strcpy(task->md5, row[6]);
    } else {
        task->md5[0] = '\0';
    }

    mysql_free_result(result);
    return ERROR_SUCCESS;
}

/**
 * @brief 更新传输进度
 * @param mysql 数据库连接指针
 * @param filename 文件名
 * @param completed_blocks 已完成分块数
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int db_update_progress(MYSQL *mysql, const char *filename, int completed_blocks) {
    char query[1024];
    sprintf(query, "UPDATE transfer_task SET completed_blocks = %d WHERE filename = '%s'", completed_blocks, filename);

    if (mysql_query(mysql, query) != 0) {
        fprintf(stderr, "mysql_query failed: %s\n", mysql_error(mysql));
        return ERROR_DATABASE;
    }

    return ERROR_SUCCESS;
}

/**
 * @brief 更新传输状态
 * @param mysql 数据库连接指针
 * @param filename 文件名
 * @param status 状态
 * @param md5 MD5值
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int db_update_status(MYSQL *mysql, const char *filename, int status, const char *md5) {
    char query[1024];
    sprintf(query, "UPDATE transfer_task SET status = %d, md5 = '%s' WHERE filename = '%s'", status, md5, filename);

    if (mysql_query(mysql, query) != 0) {
        fprintf(stderr, "mysql_query failed: %s\n", mysql_error(mysql));
        return ERROR_DATABASE;
    }

    return ERROR_SUCCESS;
}
