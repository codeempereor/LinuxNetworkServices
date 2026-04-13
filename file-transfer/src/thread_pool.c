/*****************************************************************
 > File Name:    thread_pool.c
 > Author:       三道渊
 > Description:  线程池实现
 > Created Time: 2026年04月13日 星期日 19时30分00秒
 *****************************************************************/

#include "thread_pool.h"

/**
 * @brief 工作线程函数
 * @param arg 线程参数
 * @return NULL
 */
void *worker_thread(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;

    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);

        // 等待任务队列非空
        while (pool->queue_size == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_not_empty, &pool->queue_mutex);
        }

        // 线程池关闭
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_mutex);
            pthread_exit(NULL);
        }

        // 取出任务
        Task task = pool->task_queue[pool->queue_front];
        pool->queue_front = (pool->queue_front + 1) % pool->queue_capacity;
        pool->queue_size--;

        // 通知任务队列非满
        pthread_cond_signal(&pool->queue_not_full);
        pthread_mutex_unlock(&pool->queue_mutex);

        // 处理任务
        // 这里需要根据具体的任务类型执行相应的操作
        // 暂时留空，后续在server.c中实现具体的任务处理逻辑
    }
}

/**
 * @brief 创建线程池
 * @param init_size 初始线程数
 * @param max_size 最大线程数
 * @param queue_capacity 任务队列容量
 * @return 线程池指针
 */
ThreadPool *thread_pool_create(int init_size, int max_size, int queue_capacity) {
    ThreadPool *pool = (ThreadPool *)safe_malloc(sizeof(ThreadPool));

    pool->threads = (pthread_t *)safe_malloc(sizeof(pthread_t) * max_size);
    pool->thread_count = init_size;
    pool->max_thread_count = max_size;

    pool->task_queue = (Task *)safe_malloc(sizeof(Task) * queue_capacity);
    pool->queue_capacity = queue_capacity;
    pool->queue_size = 0;
    pool->queue_front = 0;
    pool->queue_rear = 0;

    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_not_empty, NULL);
    pthread_cond_init(&pool->queue_not_full, NULL);

    pool->shutdown = 0;

    // 创建初始线程
    for (int i = 0; i < init_size; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            error_exit("pthread_create failed");
        }
    }

    return pool;
}

/**
 * @brief 销毁线程池
 * @param pool 线程池指针
 */
void thread_pool_destroy(ThreadPool *pool) {
    pool->shutdown = 1;

    // 唤醒所有线程
    pthread_cond_broadcast(&pool->queue_not_empty);

    // 等待所有线程退出
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    // 释放资源
    free(pool->threads);
    free(pool->task_queue);
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_not_empty);
    pthread_cond_destroy(&pool->queue_not_full);
    free(pool);
}

/**
 * @brief 向线程池添加任务
 * @param pool 线程池指针
 * @param task 任务指针
 * @return 成功返回ERROR_SUCCESS，失败返回错误码
 */
int thread_pool_add_task(ThreadPool *pool, Task *task) {
    pthread_mutex_lock(&pool->queue_mutex);

    // 等待任务队列非满
    while (pool->queue_size == pool->queue_capacity && !pool->shutdown) {
        pthread_cond_wait(&pool->queue_not_full, &pool->queue_mutex);
    }

    // 线程池关闭
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->queue_mutex);
        return ERROR_THREAD;
    }

    // 添加任务
    pool->task_queue[pool->queue_rear] = *task;
    pool->queue_rear = (pool->queue_rear + 1) % pool->queue_capacity;
    pool->queue_size++;

    // 通知任务队列非空
    pthread_cond_signal(&pool->queue_not_empty);
    pthread_mutex_unlock(&pool->queue_mutex);

    return ERROR_SUCCESS;
}
