-- =============================================================================
-- 项目名称: Linux磁盘文件远程检索服务
-- 文件名称: init_db.sql
-- 作    者: 三道渊
-- 创建日期: 2026-03-29
-- 最后修改: 2026-03-29
-- 版    本: v1.0.0
-- 适用平台: MySQL 8.0+
-- 功能描述:
--   1. 数据库初始化脚本，创建数据库、用户和表
-- 算法描述:
--   - 无
-- 适用场景:
--   - 数据库初始化
--   - 表结构创建
-- 版权声明: © 2026 三道渊. All rights reserved.
-- 变更记录:
--   - 2026-03-29 三道渊: 初始化文件
-- =============================================================================

-- 创建数据库
CREATE DATABASE IF NOT EXISTS file_search_db 
    CHARACTER SET utf8mb4 
    COLLATE utf8mb4_unicode_ci;

USE file_search_db;

-- 创建数据库用户
CREATE USER IF NOT EXISTS 'filesearch'@'localhost' 
    IDENTIFIED BY 'filesearch123';

-- 授予权限
GRANT ALL PRIVILEGES ON file_search_db.* 
    TO 'filesearch'@'localhost';

FLUSH PRIVILEGES;

-- 文件索引表
CREATE TABLE IF NOT EXISTS file_index (
    id INT AUTO_INCREMENT PRIMARY KEY,
    filename VARCHAR(255) NOT NULL,
    path VARCHAR(1024) NOT NULL,
    size BIGINT NOT NULL,
    mtime BIGINT NOT NULL,
    INDEX idx_filename (filename),
    INDEX idx_size (size),
    INDEX idx_mtime (mtime)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 检索记录表
CREATE TABLE IF NOT EXISTS search_record (
    id INT AUTO_INCREMENT PRIMARY KEY,
    client_ip VARCHAR(45) NOT NULL,
    search_cmd TEXT NOT NULL,
    search_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    match_count INT DEFAULT 0,
    INDEX idx_search_time (search_time),
    INDEX idx_client_ip (client_ip)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 插入测试数据（可选）
-- INSERT INTO file_index (filename, path, size, mtime) VALUES
-- ('test.txt', '/home/user/test.txt', 1024, 1690000000),
-- ('example.c', '/home/user/example.c', 2048, 1690000000);
