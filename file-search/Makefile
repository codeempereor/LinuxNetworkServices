# =============================================================================
# 项目名称: Linux磁盘文件远程检索服务
# 文件名称: Makefile
# 作    者: 三道渊
# 创建日期: 2026-03-29
# 最后修改: 2026-03-29
# 版    本: v1.0.0
# 适用平台: Linux x64
# 编译依赖: GCC, MySQL, pthread
# 功能描述:
#   1. 项目编译脚本，生成search_server和search_client
# 算法描述:
#   - 自动依赖管理：自动检测源文件依赖关系
#   - 增量编译：只编译修改过的文件
# 适用场景:
#   - 项目构建
#   - 自动化编译
# 版权声明: © 2026 三道渊. All rights reserved.
# 变更记录:
#   - 2026-03-29 三道渊: 初始化文件
# =============================================================================

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -I./include
LDFLAGS = -lpthread -lmysqlclient -lrt

# 目录设置
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# 源文件
SERVER_SRCS = $(SRC_DIR)/search_server.c \
              $(SRC_DIR)/file_index.c \
              $(SRC_DIR)/database.c \
              $(SRC_DIR)/network.c \
              $(SRC_DIR)/format.c \
              $(SRC_DIR)/utils.c

CLIENT_SRCS = $(SRC_DIR)/search_client.c \
              $(SRC_DIR)/format.c

# 目标文件
SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SERVER_SRCS))
CLIENT_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CLIENT_SRCS))

# 可执行文件
SERVER_BIN = $(BIN_DIR)/search_server
CLIENT_BIN = $(BIN_DIR)/search_client

# 默认目标
all: directories $(SERVER_BIN) $(CLIENT_BIN)

# 创建目录
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# 编译服务端
$(SERVER_BIN): $(SERVER_OBJS)
	$(CC) $(SERVER_OBJS) -o $@ $(LDFLAGS)
	@echo "Compiled: $@"

# 编译客户端
$(CLIENT_BIN): $(CLIENT_OBJS)
	$(CC) $(CLIENT_OBJS) -o $@ $(LDFLAGS)
	@echo "Compiled: $@"

# 编译规则
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned build files"

# 重新编译
rebuild: clean all

# 安装
install: all
	@cp $(SERVER_BIN) /usr/local/bin/
	@cp $(CLIENT_BIN) /usr/local/bin/
	@echo "Installed to /usr/local/bin/"

# 卸载
uninstall:
	@rm -f /usr/local/bin/search_server
	@rm -f /usr/local/bin/search_client
	@echo "Uninstalled from /usr/local/bin/"

# 运行服务端（前台模式）
run-server: all
	./$(SERVER_BIN) -f

# 运行客户端
run-client: all
	./$(CLIENT_BIN)

# 初始化数据库
db-init:
	@mysql -u root -p < sql/init_db.sql
	@echo "Database initialized"

# 帮助
help:
	@echo "Available targets:"
	@echo "  all         - Build all targets (default)"
	@echo "  clean       - Remove build files"
	@echo "  rebuild     - Clean and rebuild"
	@echo "  install     - Install binaries to /usr/local/bin"
	@echo "  uninstall   - Remove binaries from /usr/local/bin"
	@echo "  run-server  - Run server in foreground mode"
	@echo "  run-client  - Run client"
	@echo "  db-init     - Initialize database"
	@echo "  help        - Show this help"

.PHONY: all directories clean rebuild install uninstall run-server run-client db-init help
