/*****************************************************************
 > File Name:    queue.h
 > Author:       三道渊
 > Description:  队列头文件，定义队列操作函数
 > Created Time: 2026年04月14日 星期一 17时01分00秒
 *****************************************************************/

#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

/**
 * 创建队列
 * @param capacity 队列容量
 * @return 队列实例
 */
LogQueue* queue_create(int capacity);

/**
 * 销毁队列
 * @param queue 队列实例
 */
void queue_destroy(LogQueue* queue);

/**
 * 入队
 * @param queue 队列实例
 * @param log 日志条目
 * @return 错误码
 */
int queue_enqueue(LogQueue* queue, LogEntry* log);

/**
 * 出队
 * @param queue 队列实例
 * @param log 日志条目指针
 * @return 错误码
 */
int queue_dequeue(LogQueue* queue, LogEntry* log);

/**
 * 获取队列大小
 * @param queue 队列实例
 * @return 队列大小
 */
int queue_size(LogQueue* queue);

/**
 * 检查队列是否为空
 * @param queue 队列实例
 * @return 是否为空
 */
int queue_is_empty(LogQueue* queue);

/**
 * 检查队列是否已满
 * @param queue 队列实例
 * @return 是否已满
 */
int queue_is_full(LogQueue* queue);

#endif /* QUEUE_H */