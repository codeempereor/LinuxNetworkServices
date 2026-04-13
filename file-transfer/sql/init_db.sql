-- 创建数据库
CREATE DATABASE IF NOT EXISTS transfer_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE transfer_db;

-- 创建传输任务表
CREATE TABLE IF NOT EXISTS transfer_task (
    id INT AUTO_INCREMENT PRIMARY KEY,
    filename VARCHAR(255) UNIQUE NOT NULL,
    file_size BIGINT NOT NULL,
    total_blocks INT NOT NULL,
    completed_blocks INT NOT NULL DEFAULT 0,
    client_ip VARCHAR(50) NOT NULL,
    status TINYINT NOT NULL DEFAULT 0, -- 0=传输中/1=完成/2=失败
    md5 VARCHAR(32) DEFAULT NULL,
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    update_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 创建索引
CREATE INDEX idx_filename ON transfer_task(filename);
CREATE INDEX idx_status ON transfer_task(status);
