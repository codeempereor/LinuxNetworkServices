/*****************************************************************
 > File Name:    queue.c
 > Author:       三道渊
 > Description:  队列实现文件
 > Created Time: 2026年04月14日 星期一 17时04分00秒
 *****************************************************************/

#include "queue.h"

/**
 * 创建队列
 * @param capacity 队列容量
 * @return 队列实例
 */
LogQueue* queue_create(int capacity) {
    LogQueue* queue = (LogQueue*)malloc(sizeof(LogQueue));
    if (!queue) {
        perror("malloc failed");
        return NULL;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->capacity = capacity;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    pthread_cond_init(&queue->not_empty, NULL);

    return queue;
}

/**
 * 销毁队列
 * @param queue 队列实例
 */
void queue_destroy(LogQueue* queue) {
    if (!queue) {
        return;
    }

    // 释放所有节点
    QueueNode* current = queue->head;
    while (current) {
        QueueNode* next = current->next;
        free(current);
        current = next;
    }

    // 销毁锁和条件变量
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_full);
    pthread_cond_destroy(&queue->not_empty);

    free(queue);
}

/**
 * 入队
 * @param queue 队列实例
 * @param log 日志条目
 * @return 错误码
 */
int queue_enqueue(LogQueue* queue, LogEntry* log) {
    if (!queue || !log) {
        return ERROR_SEND;
    }

    pthread_mutex_lock(&queue->mutex);

    // 等待队列非满
    while (queue->size >= queue->capacity) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }

    // 创建新节点
    QueueNode* node = (QueueNode*)malloc(sizeof(QueueNode));
    if (!node) {
        pthread_mutex_unlock(&queue->mutex);
        return ERROR_SEND;
    }

    node->log = *log;
    node->next = NULL;

    // 入队
    if (!queue->tail) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }

    queue->size++;

    // 通知非空
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);

    return ERROR_SUCCESS;
}

/**
 * 出队
 * @param queue 队列实例
 * @param log 日志条目指针
 * @return 错误码
 */
int queue_dequeue(LogQueue* queue, LogEntry* log) {
    if (!queue || !log) {
        return ERROR_RECV;
    }

    pthread_mutex_lock(&queue->mutex);

    // 等待队列非空
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }

    // 出队
    QueueNode* node = queue->head;
    *log = node->log;

    if (queue->head == queue->tail) {
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        queue->head = queue->head->next;
    }

    free(node);
    queue->size--;

    // 通知非满
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);

    return ERROR_SUCCESS;
}

/**
 * 获取队列大小
 * @param queue 队列实例
 * @return 队列大小
 */
int queue_size(LogQueue* queue) {
    if (!queue) {
        return 0;
    }

    pthread_mutex_lock(&queue->mutex);
    int size = queue->size;
    pthread_mutex_unlock(&queue->mutex);

    return size;
}

/**
 * 检查队列是否为空
 * @param queue 队列实例
 * @return 是否为空
 */
int queue_is_empty(LogQueue* queue) {
    return queue_size(queue) == 0;
}

/**
 * 检查队列是否已满
 * @param queue 队列实例
 * @return 是否已满
 */
int queue_is_full(LogQueue* queue) {
    if (!queue) {
        return 0;
    }

    pthread_mutex_lock(&queue->mutex);
    int full = (queue->size >= queue->capacity);
    pthread_mutex_unlock(&queue->mutex);

    return full;
}