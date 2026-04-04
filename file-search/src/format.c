/*****************************************************************
 > File Name:    format.c
 > Author:       三道渊
 > Description:  结果格式化模块，实现数据格式转换和表格输出
 > Created Time: 2026年03月03日 星期六 13时01分11秒
 *****************************************************************/

#include "../include/common.h"

/**
 * @brief 格式化文件大小（自动转换为KB/MB/GB）
 * @param size_bytes 文件大小（字节）
 * @param output 输出缓冲区
 * @param output_size 缓冲区大小
 */
void format_size(off_t size_bytes, char *output, size_t output_size)
{
    if (size_bytes < 1024)
    {
        snprintf(output, output_size, "%ldB", (long)size_bytes);
    }
    else if (size_bytes < 1024 * 1024)
    {
        snprintf(output, output_size, "%.1fKB", (double)size_bytes / 1024);
    }
    else if (size_bytes < 1024 * 1024 * 1024)
    {
        snprintf(output, output_size, "%.1fMB", (double)size_bytes / (1024 * 1024));
    }
    else
    {
        snprintf(output, output_size, "%.1fGB", (double)size_bytes / (1024 * 1024 * 1024));
    }
}

/**
 * @brief 格式化时间戳为标准日期时间格式
 * @param mtime 修改时间戳
 * @param output 输出缓冲区
 * @param output_size 缓冲区大小
 */
void format_time(time_t mtime, char *output, size_t output_size)
{
    struct tm *tm_info = localtime(&mtime);
    if (tm_info != NULL)
    {
        strftime(output, output_size, "%Y-%m-%d %H:%M:%S", tm_info);
    }
    else
    {
        strncpy(output, "Invalid", output_size - 1);
        output[output_size - 1] = '\0';
    }
}

/**
 * @brief 打印表格头部
 */
void print_table_header(void)
{
    printf("+----------------+----------------------+--------+----------+---------------------+\n");
    printf("| %-14s | %-20s | %-6s | %-19s |\n", "filename", "path", "size", "mtime");
    printf("+----------------+----------------------+--------+----------+---------------------+\n");
}

/**
 * @brief 打印表格行
 * @param file 文件信息结构
 */
void print_table_row(const FileInfo *file)
{
    char size_str[16];
    char time_str[32];
    
    format_size(file->size, size_str, sizeof(size_str));
    format_time(file->mtime, time_str, sizeof(time_str));
    
    printf("| %-14s | %-20s | %-6s | %-19s |\n",
           file->filename,
           file->path,
           size_str,
           time_str);
}

/**
 * @brief 打印表格底部
 * @param total_count 总行数
 */
void print_table_footer(int total_count)
{
    printf("+----------------+----------------------+--------+----------+---------------------+\n");
    printf("Total: %d files\n", total_count);
}
