/*****************************************************************
 > File Name:    thread_pool.h
 > Author:       三道渊
 > Description:  线程池相关头文件
 > Created Time: 2026年04月13日 星期日 19时30分00秒
 *****************************************************************/

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "common.h"

/**
 * 线程池结构体
 */
typedef struct {
    pthread_t *threads;         /* 线程数组 */
    int thread_count;           /* 当前线程数 */
    int max_thread_count;       /* 最大线程数 */
    Task *task_queue;           /* 任务队列 */
    int queue_capacity;         /* 队列容量 */
    int queue_size;             /* 当前队列大小 */
    int queue_front;            /* 队列前端索引 */
    int queue_rear;             /* 队列后端索引 */
    pthread_mutex_t queue_mutex; /* 队列互斥锁 */
    pthread_cond_t queue_not_empty; /* 队列非空条件变量 */
    pthread_cond_t queue_not_full;  /* 队列非满条件变量 */
    int shutdown;               /* 线程池关闭标志 */
} ThreadPool;

/**
 * 函数声明
 */
/**
 * @brief 创建线程池
 * @param init_size 初始线程数
 * @param max_size 最大线程数
 * @param queue_capacity 任务队列容量
 * @return 线程池指针
 */
ThreadPool *thread_pool_create(int init_size, int max_size, int queue_capacity);

/**
 * @brief 销毁线程池
 * @param pool 线程池指针
 */
void thread_pool_destroy(ThreadPool *pool);

/**
 * @brief 向线程池添加任务
 * @param pool 线程池指针
 * @param task 任务指针
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int thread_pool_add_task(ThreadPool *pool, Task *task);

#endif // THREAD_POOL_H
