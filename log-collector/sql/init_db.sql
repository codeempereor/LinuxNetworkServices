-- 创建数据库
CREATE DATABASE IF NOT EXISTS log_collector CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 使用数据库
USE log_collector;

-- 创建debug_log表
CREATE TABLE IF NOT EXISTS debug_log (
    id INT PRIMARY KEY AUTO_INCREMENT,
    timestamp BIGINT NOT NULL,
    client_ip VARCHAR(16) NOT NULL,
    module VARCHAR(64) NOT NULL,
    content VARCHAR(1024) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建info_log表
CREATE TABLE IF NOT EXISTS info_log (
    id INT PRIMARY KEY AUTO_INCREMENT,
    timestamp BIGINT NOT NULL,
    client_ip VARCHAR(16) NOT NULL,
    module VARCHAR(64) NOT NULL,
    content VARCHAR(1024) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建warn_log表
CREATE TABLE IF NOT EXISTS warn_log (
    id INT PRIMARY KEY AUTO_INCREMENT,
    timestamp BIGINT NOT NULL,
    client_ip VARCHAR(16) NOT NULL,
    module VARCHAR(64) NOT NULL,
    content VARCHAR(1024) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建error_log表
CREATE TABLE IF NOT EXISTS error_log (
    id INT PRIMARY KEY AUTO_INCREMENT,
    timestamp BIGINT NOT NULL,
    client_ip VARCHAR(16) NOT NULL,
    module VARCHAR(64) NOT NULL,
    content VARCHAR(1024) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建索引
CREATE INDEX idx_debug_timestamp ON debug_log(timestamp);
CREATE INDEX idx_debug_module ON debug_log(module);
CREATE INDEX idx_info_timestamp ON info_log(timestamp);
CREATE INDEX idx_info_module ON info_log(module);
CREATE INDEX idx_warn_timestamp ON warn_log(timestamp);
CREATE INDEX idx_warn_module ON warn_log(module);
CREATE INDEX idx_error_timestamp ON error_log(timestamp);
CREATE INDEX idx_error_module ON error_log(module);

-- 授权用户
GRANT ALL PRIVILEGES ON log_collector.* TO 'root'@'localhost' IDENTIFIED BY '1';
FLUSH PRIVILEGES;