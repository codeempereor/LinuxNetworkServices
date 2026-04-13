/*****************************************************************
 > File Name:    utils.c
 > Author:       三道渊
 > Description:  工具函数实现
 > Created Time: 2026年04月13日 星期日 19时30分00秒
 *****************************************************************/

#include "common.h"

/**
 * @brief 错误处理函数
 * @param msg 错误信息
 */
void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/**
 * @brief 安全的内存分配
 * @param size 内存大小
 * @return 分配的内存指针
 */
void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        error_exit("malloc failed");
    }
    return ptr;
}

/**
 * @brief 计算文件的MD5值
 * @param file_path 文件路径
 * @return MD5字符串
 */
char *calculate_md5(const char *file_path) {
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return NULL;
    }

    char *buffer = safe_malloc(BLOCK_SIZE);
    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);

    ssize_t read_size;
    while ((read_size = read(fd, buffer, BLOCK_SIZE)) > 0) {
        MD5_Update(&md5_ctx, buffer, read_size);
    }

    unsigned char md5_digest[MD5_DIGEST_LENGTH];
    MD5_Final(md5_digest, &md5_ctx);

    close(fd);
    free(buffer);

    char *md5_str = safe_malloc(33);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(&md5_str[i*2], "%02x", md5_digest[i]);
    }
    md5_str[32] = '\0';

    return md5_str;
}

/**
 * @brief 获取文件大小
 * @param file_path 文件路径
 * @return 文件大小
 */
long long get_file_size(const char *file_path) {
    struct stat st;
    if (stat(file_path, &st) < 0) {
        return -1;
    }
    return st.st_size;
}

/**
 * @brief 计算总分块数
 * @param file_size 文件大小
 * @return 分块数
 */
int calculate_total_blocks(long long file_size) {
    return (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}
