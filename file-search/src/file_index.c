/*****************************************************************
 > File Name:    file_index.c
 > Author:       三道渊
 > Description:  文件索引构建模块，实现二叉树索引和异步更新
 > Created Time: 2026年03月03日 星期六 13时01分11秒
 *****************************************************************/

#include "../include/common.h"

/* 全局变量定义 */
TreeNode *g_file_index_root = NULL;
pthread_mutex_t g_index_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief 创建二叉树节点
 * @param filename 文件名
 * @param path 绝对路径
 * @param size 文件大小
 * @param mtime 修改时间
 * @return 新创建的节点指针
 */
TreeNode* create_tree_node(const char *filename, const char *path, off_t size, time_t mtime)
{
    TreeNode *node = (TreeNode*)malloc(sizeof(TreeNode));
    if (node == NULL)
    {
        write_log("Failed to allocate memory for tree node");
        return NULL;
    }
    
    strncpy(node->filename, filename, MAX_FILENAME_LENGTH - 1);
    node->filename[MAX_FILENAME_LENGTH - 1] = '\0';
    strncpy(node->path, path, MAX_PATH_LENGTH - 1);
    node->path[MAX_PATH_LENGTH - 1] = '\0';
    node->size = size;
    node->mtime = mtime;
    node->left = NULL;
    node->right = NULL;
    
    return node;
}

/**
 * @brief 插入节点到二叉树（按filename字典序）
 * @param root 根节点
 * @param new_node 新节点
 * @return 根节点指针
 */
TreeNode* insert_tree_node(TreeNode *root, TreeNode *new_node)
{
    if (root == NULL)
    {
        return new_node;
    }
    
    int cmp = strcmp(new_node->filename, root->filename);
    if (cmp < 0)
    {
        root->left = insert_tree_node(root->left, new_node);
    }
    else if (cmp > 0)
    {
        root->right = insert_tree_node(root->right, new_node);
    }
    /* 如果相等，不插入（避免重复） */
    
    return root;
}

/**
 * @brief 释放二叉树内存
 * @param root 根节点
 */
void free_tree(TreeNode *root)
{
    if (root == NULL)
    {
        return;
    }
    
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

/**
 * @brief 在二叉树中搜索指定文件名的节点
 * @param root 根节点
 * @param filename 文件名
 * @return 找到的节点指针，未找到返回NULL
 */
TreeNode* search_tree(TreeNode *root, const char *filename)
{
    if (root == NULL)
    {
        return NULL;
    }
    
    int cmp = strcmp(filename, root->filename);
    if (cmp == 0)
    {
        return root;
    }
    else if (cmp < 0)
    {
        return search_tree(root->left, filename);
    }
    else
    {
        return search_tree(root->right, filename);
    }
}

/**
 * @brief 遍历二叉树，收集符合条件的文件
 * @param root 根节点
 * @param result 结果结构
 * @param cond 检索条件
 */
void traverse_tree(TreeNode *root, SearchResult *result, SearchCondition *cond)
{
    if (root == NULL)
    {
        return;
    }
    
    /* 中序遍历：左-根-右 */
    traverse_tree(root->left, result, cond);
    
    /* 检查当前节点是否符合条件 */
    int match = 1;
    
    /* 精确匹配文件名 */
    if (cond->has_filename && strcmp(root->filename, cond->filename) != 0)
    {
        match = 0;
    }
    
    /* 模糊匹配文件名 */
    if (match && cond->has_filename_like)
    {
        if (strstr(root->filename, cond->filename_like) == NULL)
        {
            match = 0;
        }
    }
    
    /* 文件大小条件 */
    if (match && cond->has_size_gt && root->size <= cond->size_gt)
    {
        match = 0;
    }
    
    /* 修改时间条件 */
    if (match && cond->has_mtime_gt && root->mtime <= cond->mtime_gt)
    {
        match = 0;
    }
    
    /* 正则表达式匹配 */
    if (match && cond->has_regex)
    {
        regex_t regex;
        if (regcomp(&regex, cond->regex_pattern, REG_EXTENDED) == 0)
        {
            if (regexec(&regex, root->filename, 0, NULL, 0) != 0)
            {
                match = 0;
            }
            regfree(&regex);
        }
    }
    
    /* 如果匹配，添加到结果 */
    if (match)
    {
        if (result->count >= result->capacity)
        {
            /* 扩展数组容量 */
            int new_capacity = result->capacity * 2;
            FileInfo *new_files = realloc(result->files, new_capacity * sizeof(FileInfo));
            if (new_files != NULL)
            {
                result->files = new_files;
                result->capacity = new_capacity;
            }
        }
        
        if (result->count < result->capacity)
        {
            strncpy(result->files[result->count].filename, root->filename, MAX_FILENAME_LENGTH - 1);
            strncpy(result->files[result->count].path, root->path, MAX_PATH_LENGTH - 1);
            result->files[result->count].size = root->size;
            result->files[result->count].mtime = root->mtime;
            result->count++;
        }
    }
    
    traverse_tree(root->right, result, cond);
}

/**
 * @brief 递归遍历目录，构建文件索引
 * @param dir_path 目录路径
 * @param root 根节点指针的指针
 * @return 处理的文件数量
 */
static int traverse_directory(const char *dir_path, TreeNode **root)
{
    int count = 0;
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        write_log("Failed to open directory: %s", dir_path);
        return 0;
    }
    
    struct dirent *entry;
    struct stat file_stat;
    char full_path[MAX_PATH_LENGTH];
    
    while ((entry = readdir(dir)) != NULL)
    {
        /* 跳过当前目录和父目录 */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        
        /* 构建完整路径 */
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        /* 获取文件状态 */
        if (stat(full_path, &file_stat) < 0)
        {
            continue;
        }
        
        /* 只处理普通文件 */
        if (S_ISREG(file_stat.st_mode))
        {
            TreeNode *node = create_tree_node(entry->d_name, full_path, 
                                              file_stat.st_size, file_stat.st_mtime);
            if (node != NULL)
            {
                *root = insert_tree_node(*root, node);
                count++;
            }
        }
        /* 递归处理子目录 */
        else if (S_ISDIR(file_stat.st_mode))
        {
            count += traverse_directory(full_path, root);
        }
    }
    
    closedir(dir);
    return count;
}

/**
 * @brief 构建文件索引
 * @param base_dir 基础目录路径
 * @return 处理的文件数量
 */
int build_file_index(const char *base_dir)
{
    pthread_mutex_lock(&g_index_mutex);
    
    /* 释放旧索引 */
    if (g_file_index_root != NULL)
    {
        free_tree(g_file_index_root);
        g_file_index_root = NULL;
    }
    
    /* 构建新索引 */
    int count = traverse_directory(base_dir, &g_file_index_root);
    
    pthread_mutex_unlock(&g_index_mutex);
    
    write_log("Built file index: %d files from %s", count, base_dir);
    return count;
}

/**
 * @brief 索引更新线程函数
 * @param arg 线程参数（未使用）
 * @return NULL
 */
void* index_update_thread(void *arg)
{
    (void)arg;  /* 避免未使用参数警告 */
    
    while (1)
    {
        sleep(INDEX_UPDATE_INTERVAL);
        
        write_log("Starting periodic index update...");
        
        /* 构建新索引 */
        int count = build_file_index(SEARCH_BASE_DIR);
        
        /* 更新数据库 */
        pthread_mutex_lock(&g_index_mutex);
        if (g_file_index_root != NULL)
        {
            db_insert_file_index(g_file_index_root);
        }
        pthread_mutex_unlock(&g_index_mutex);
        
        write_log("Index update completed: %d files", count);
    }
    
    return NULL;
}
