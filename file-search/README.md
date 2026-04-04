# Linux磁盘文件远程检索服务

## 项目概述

本项目是一个基于C语言实现的Linux文件远程检索服务，支持通过TCP协议进行远程文件检索，具有高性能、高并发、实时索引更新等特点。

## 功能特性

### 1. 文件索引构建模块
- 服务端启动时遍历 `/home/` 目录构建二叉树索引
- 索引包含：filename（文件名）、path（绝对路径）、size（文件大小）、mtime（修改时间）
- 构建时间 ≤ 30秒（针对1000个文件）
- 异步线程每5分钟自动更新索引

### 2. 检索请求处理模块
- 支持4种检索条件组合：
  - 精确查询：`SEARCH filename=test.txt`
  - 模糊查询：`SEARCH filename_like=%test%`
  - 多条件：`SEARCH filename_like=%.txt% size>1024 mtime>1690000000`
  - 正则匹配：`SEARCH regex=^test.*.txt$`
- 优先查询内存二叉树索引，查询不到再查MySQL
- 检索响应时间 ≤ 1秒（针对1000个文件）
- 比Linux原生find指令快30%以上

### 3. 网络通信模块
- TCP服务端监听端口7777
- 支持最多20个并发客户端连接
- 每个客户端连接创建独立线程处理
- 客户端支持命令行交互

### 4. 数据存储模块
- MySQL数据库持久化存储
- file_index表：存储文件索引
- search_record表：存储检索记录
- 支持热度统计和过期记录清理

### 5. 结果格式化模块
- 文件大小自动转换（B/KB/MB/GB）
- 时间戳转换为标准日期时间格式
- 表格格式展示检索结果

## 技术架构

```
┌─────────────────────────────────────────────────────────────┐
│                    Linux File Search Service                 │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │   Client     │  │   Client     │  │   Client     │       │
│  │  (TCP:7777)  │  │  (TCP:7777)  │  │  (TCP:7777)  │       │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘       │
│         │                  │                  │              │
│         └──────────────────┼──────────────────┘              │
│                            │                                 │
│  ┌─────────────────────────┴─────────────────────────────┐   │
│  │                    Server (Port 7777)                  │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐    │   │
│  │  │ Main Thread │  │ Index Thread│  │ Clean Thread│    │   │
│  │  │ (Accept)    │  │ (Update)    │  │ (Cleanup)   │    │   │
│  │  └──────┬──────┘  └─────────────┘  └─────────────┘    │   │
│  │         │                                             │   │
│  │  ┌──────┴──────┐  ┌─────────────┐  ┌─────────────┐    │   │
│  │  │ Client      │  │ Client      │  │ Client      │    │   │
│  │  │ Handler     │  │ Handler     │  │ Handler     │    │   │
│  │  │ (Thread)    │  │ (Thread)    │  │ (Thread)    │    │   │
│  │  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘    │   │
│  │         │                │                │           │   │
│  │         └────────────────┼────────────────┘           │   │
│  │                          │                            │   │
│  │  ┌───────────────────────┴────────────────────────┐   │   │
│  │  │              Binary Tree Index (Memory)         │   │   │
│  │  │  ┌──────────────────────────────────────────┐   │   │   │
│  │  │  │  Root                                    │   │   │   │
│  │  │  │  ├── Left Subtree (filename < root)      │   │   │   │
│  │  │  │  └── Right Subtree (filename > root)     │   │   │   │
│  │  │  └──────────────────────────────────────────┘   │   │   │
│  │  └─────────────────────────────────────────────────┘   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                            │                                   │
│  ┌─────────────────────────┴─────────────────────────────┐     │
│  │                    MySQL Database                      │     │
│  │  ┌─────────────┐  ┌─────────────┐                     │     │
│  │  │ file_index  │  │search_record│                     │     │
│  │  │  (索引表)    │  │  (记录表)    │                     │     │
│  │  └─────────────┘  └─────────────┘                     │     │
│  └─────────────────────────────────────────────────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

## 项目结构

```
file_search_service/
├── include/
│   └── common.h          # 公共头文件
├── src/
│   ├── search_server.c   # 服务端主程序
│   ├── search_client.c   # 客户端程序
│   ├── file_index.c      # 文件索引模块
│   ├── database.c        # 数据库模块
│   ├── network.c         # 网络通信模块
│   ├── format.c          # 结果格式化模块
│   └── utils.c           # 工具函数模块
├── sql/
│   └── init_db.sql       # 数据库初始化脚本
├── logs/
│   └── file_search.log   # 日志文件
├── Makefile              # 编译脚本
└── README.md             # 项目文档
```

## 编译安装

### 依赖安装

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y gcc make libmysqlclient-dev

# CentOS/RHEL
sudo yum install -y gcc make mysql-devel
```

### 编译项目

```bash
cd file_search_service
make all
```

### 安装到系统

```bash
sudo make install
```

## 数据库配置

### 初始化数据库

```bash
mysql -u root -p < sql/init_db.sql
```

### 数据库配置（include/common.h）

```c
#define DB_HOST "localhost"
#define DB_USER "filesearch"
#define DB_PASS "filesearch123"
#define DB_NAME "file_search_db"
```

## 使用说明

### 启动服务端

```bash
# 后台模式（守护进程）
sudo search_server

# 前台模式（调试）
sudo search_server -f
```

### 启动客户端

```bash
# 连接本地服务器
search_client

# 连接远程服务器
search_client -h 192.168.1.100 -p 7777
```

### 客户端命令

```
# 精确查询
SEARCH filename=test.txt

# 模糊查询
SEARCH filename_like=%test%

# 多条件查询
SEARCH filename_like=%.c% size>1024 mtime>1690000000

# 正则匹配
SEARCH regex=^test.*.txt$

# 统计热度
STAT test.txt

# 帮助
help

# 退出
quit
```

## 性能指标

- 索引构建时间：≤ 30秒（1000个文件）
- 检索响应时间：≤ 1秒（1000个文件）
- 并发连接数：20个
- 索引更新间隔：5分钟
- 记录保留时间：30天

## 算法说明

### 1. 二叉搜索树（BST）
- **用途**：内存中的文件索引存储
- **特点**：按filename字典序排序，支持快速查找
- **时间复杂度**：查找O(log n)，插入O(log n)

### 2. 多线程并发
- **用途**：支持多个客户端同时连接
- **特点**：每个连接创建独立线程，互斥锁保护共享资源
- **优势**：提高并发处理能力

### 3. 异步索引更新
- **用途**：定期更新文件索引
- **特点**：后台线程执行，不阻塞检索服务
- **优势**：保证数据实时性，不影响服务性能

## 安全特性

- 路径合法性检查：禁止访问 `/root/`, `/proc/`, `/sys/`, `/dev/` 等系统目录
- SQL注入防护：使用预处理语句
- 并发安全：互斥锁保护共享资源
- 日志记录：记录所有操作便于审计

## 日志文件

日志位置：`/var/log/file_search.log`

日志格式：`[YYYY-MM-DD HH:MM:SS] 日志内容`

## 测试用例

```bash
# 1. 检索所有.c文件
SEARCH filename_like=%.c%

# 2. 检索大于1KB的.c文件
SEARCH filename_like=%.c% size>1024

# 3. 检索最近修改的文件
SEARCH mtime>1690000000

# 4. 统计文件检索次数
STAT test.txt
```

## 故障排除

### 1. 编译错误

```bash
# 检查依赖
sudo apt-get install -y gcc make libmysqlclient-dev

# 重新编译
make clean && make all
```

### 2. 数据库连接失败

```bash
# 检查MySQL服务
sudo systemctl status mysql

# 检查数据库用户
mysql -u filesearch -p file_search_db
```

### 3. 端口被占用

```bash
# 查看端口占用
sudo netstat -tlnp | grep 7777

# 结束占用进程
sudo kill -9 <PID>
```

## 作者信息

- 作者：三道渊
- 创建日期：2026-03-29
- 版本：v1.0.0
- 许可证：MIT License

## 更新记录

- 2026-03-29: 初始化项目，实现基本功能
