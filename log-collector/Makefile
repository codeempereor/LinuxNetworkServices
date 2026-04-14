# 多线程网络日志收集系统 Makefile

# 目录设置
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
CONFIG_DIR = config
SQL_DIR = sql

# 编译器
CC = gcc
CFLAGS = -Wall -Wextra -g -I$(INCLUDE_DIR)
LDFLAGS = -lpthread -lmysqlclient

# 源文件
CLIENT_SRCS = $(SRC_DIR)/log_client.c
SERVER_SRCS = $(SRC_DIR)/log_server.c $(SRC_DIR)/queue.c $(SRC_DIR)/database.c

# 目标文件
CLIENT_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))
SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))

# 可执行文件
CLIENT_BIN = $(BIN_DIR)/log_client
SERVER_BIN = $(BIN_DIR)/log_server

# 创建目录
.PHONY: directories

directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# 编译客户端
$(CLIENT_BIN): directories $(CLIENT_OBJS)
	$(CC) $(CLIENT_OBJS) -o $@ $(LDFLAGS)

# 编译服务端
$(SERVER_BIN): directories $(SERVER_OBJS)
	$(CC) $(SERVER_OBJS) -o $@ $(LDFLAGS)

# 编译目标文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译所有
.PHONY: all

all: $(CLIENT_BIN) $(SERVER_BIN)

# 清理
.PHONY: clean

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# 安装
.PHONY: install

install:
	@mkdir -p /usr/local/bin
	cp $(CLIENT_BIN) /usr/local/bin/
	cp $(SERVER_BIN) /usr/local/bin/
	@mkdir -p /etc/log-collector
	cp $(CONFIG_DIR)/client.conf /etc/log-collector/
	cp $(CONFIG_DIR)/server.conf /etc/log-collector/

# 卸载
.PHONY: uninstall

uninstall:
	rm -f /usr/local/bin/log_client
	rm -f /usr/local/bin/log_server
	rm -rf /etc/log-collector

# 数据库初始化
.PHONY: db-init

db-init:
	mysql -u root -p < $(SQL_DIR)/init_db.sql

# 运行服务端
.PHONY: run-server

run-server:
	$(SERVER_BIN) $(CONFIG_DIR)/server.conf

# 运行客户端
.PHONY: run-client

run-client:
	$(CLIENT_BIN) $(CONFIG_DIR)/client.conf

# 帮助
.PHONY: help

help:
	@echo "多线程网络日志收集系统 Makefile"
	@echo "================================"
	@echo "make all          - 编译所有可执行文件"
	@echo "make clean        - 清理编译结果"
	@echo "make install      - 安装到系统"
	@echo "make uninstall    - 从系统卸载"
	@echo "make db-init      - 初始化数据库"
	@echo "make run-server   - 运行服务端"
	@echo "make run-client   - 运行客户端"
	@echo "make help         - 显示帮助信息"