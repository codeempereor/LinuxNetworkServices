# 📡 LinuxNetworkServices

一个基于 Linux 平台的网络服务集合，包含多种实用的网络工具和服务。

## 📁 项目结构

```
LinuxNetworkServices/
├── file-search/          # Linux 磁盘文件远程检索服务
├── file-transfer/        # 多线程断点续传网络文件传输工具
├── log-collector/        # 分布式多线程网络日志收集系统
├── README.md             # 项目说明
└── .gitignore            # Git 忽略规则
```

## 🚀 项目列表

### 1. file-search - Linux 磁盘文件远程检索服务
**类型**: 远程文件检索服务  
**技术栈**: C / Linux / MySQL / TCP/IP

**特性**:
- 🔍 高性能文件索引和检索
- 🌐 基于 TCP 协议的远程访问
- 📊 多条件组合查询（精确、模糊、正则）
- ⚡ 内存二叉树索引，检索速度快
- 🔄 异步索引更新机制
- 🔐 安全的路径检查和 SQL 注入防护
- 📈 支持热度统计和记录管理

**使用方式**:
```bash
# 启动服务端
sudo search_server

# 客户端连接
search_client -h 192.168.1.100 -p 7777

# 检索命令
SEARCH filename=test.txt
SEARCH filename_like=%.c% size>1024
```

### 2. file-transfer - 多线程断点续传网络文件传输工具
**类型**: 文件传输服务  
**技术栈**: C / Linux / MySQL / TCP/IP / 多线程

**特性**:
- 📤 多线程并发传输，速度快
- 🔄 断点续传，支持传输中断后从断点处继续
- 📊 传输进度实时显示
- ⚡ 固定4MB分块，优化传输效率
- 🔐 MD5文件完整性校验
- 🗃️ MySQL存储传输任务信息
- 🔍 传输记录查询和管理

**使用方式**:
```bash
# 启动服务端
./bin/transfer_server

# 上传文件
./bin/transfer_client upload local_file 127.0.0.1:8888

# 下载文件
./bin/transfer_client download remote_file local_path 127.0.0.1:8888

# 查询文件状态
./bin/transfer_client query remote_file 127.0.0.1:8888
```

### 3. log-collector - 分布式多线程网络日志收集系统
**类型**: 日志收集服务  
**技术栈**: C / Linux / MySQL / TCP/IP / UDP / 多线程

**特性**:
- 📥 多协议支持（TCP/UDP）
- 🔄 服务端离线时缓存最近100条日志
- 🎯 动态日志过滤（按级别）
- 🔍 按模块名分类存储
- ⚡ 生产消费者模型，并发处理
- 🗃️ MySQL分表存储（按日志级别）
- 📊 支持多种检索方式
- 📤 日志导出功能

**使用方式**:
```bash
# 启动服务端
./bin/log_server config/server.conf

# 客户端集成API
log_send(DEBUG, "module", "content");
log_send(INFO, "module", "content");
log_send(WARN, "module", "content");
log_send(ERROR, "module", "content");

# 服务端命令
filter WARN ERROR  # 仅接收WARN和ERROR级日志
filter all         # 取消过滤
export error 1690000000 1690000100  # 导出指定时间范围的错误日志
```

## 🛠️ 环境要求

- **操作系统**: Linux (Ubuntu/Debian/CentOS)
- **编译器**: GCC
- **依赖**: MySQL, make
- **网络**: TCP/IP 网络环境

## 📦 安装部署

### 编译安装
```bash
# 进入项目目录
cd LinuxNetworkServices/file-search

# 编译
sudo make all

# 安装
sudo make install
```

### 数据库配置
```bash
# 初始化数据库
mysql -u root -p < sql/init_db.sql
```

## 🔧 技术架构

### file-search 技术架构
| 模块 | 功能 | 技术实现 |
|------|------|----------|
| 文件索引 | 构建和管理文件索引 | 二叉搜索树 |
| 网络通信 | 处理客户端连接 | TCP 多线程 |
| 数据存储 | 持久化存储 | MySQL |
| 结果格式化 | 格式化检索结果 | 自定义格式化 |
| 安全防护 | 路径检查和 SQL 注入防护 | 安全检查机制 |

### file-transfer 技术架构
| 模块 | 功能 | 技术实现 |
|------|------|----------|
| 网络通信 | 处理文件传输 | TCP 套接字 |
| 文件分块 | 大文件分块处理 | 固定4MB分块 |
| 断点续传 | 传输中断后续传 | 数据库记录 |
| 线程池 | 并发传输管理 | 多线程池 |
| 数据存储 | 传输任务管理 | MySQL |
| 完整性校验 | 文件完整性检查 | MD5算法 |

### log-collector 技术架构
| 模块 | 功能 | 技术实现 |
|------|------|----------|
| 日志采集 | 客户端日志发送 | TCP/UDP 协议 |
| 日志接收 | 服务端接收日志 | 多线程接收 |
| 日志过滤 | 动态过滤规则 | 命令行交互 |
| 并发处理 | 生产消费者模型 | 线程池 + 队列 |
| 数据存储 | 分表存储日志 | MySQL 分表 |
| 日志导出 | 导出指定日志 | 文件IO操作 |

## 📊 性能指标

### file-search 性能指标
| 指标 | 数值 |
|------|------|
| 索引构建时间 | ≤ 30秒（1000个文件） |
| 检索响应时间 | ≤ 1秒（1000个文件） |
| 并发连接数 | 20个 |
| 索引更新间隔 | 5分钟 |
| 记录保留时间 | 30天 |

### file-transfer 性能指标
| 指标 | 数值 |
|------|------|
| 传输速度 | ≥ 10MB/s（局域网） |
| 并发传输数 | 10个 |
| 最大文件大小 | 无限制 |
| 分块大小 | 4MB |
| 内存使用 | ≤ 10MB |

### log-collector 性能指标
| 指标 | 数值 |
|------|------|
| 日志处理速度 | ≥ 100条/秒 |
| 并发客户端数 | 50个 |
| 队列容量 | 1000条 |
| 缓存日志数 | 100条（客户端） |
| 消费者线程数 | 2个 |

## 🎯 应用场景

### file-search 应用场景
1. **远程文件管理**: 远程服务器文件检索
2. **自动化运维**: 批量文件查找和管理
3. **文件监控**: 实时监控文件变化
4. **数据统计**: 文件热度分析和使用情况统计
5. **开发工具**: 代码文件快速检索

### file-transfer 应用场景
1. **大文件传输**: 高效传输大文件
2. **网络不稳定环境**: 断点续传确保传输成功
3. **批量文件传输**: 多线程并发传输
4. **跨网络传输**: 局域网和广域网传输
5. **文件备份**: 定期备份重要文件

### log-collector 应用场景
1. **分布式系统日志**: 集中收集多服务器日志
2. **实时监控**: 实时监控系统运行状态
3. **故障排查**: 快速定位和分析故障
4. **性能分析**: 分析系统性能瓶颈
5. **安全审计**: 记录和分析安全事件

## 🔮 未来计划

- [ ] 添加更多网络服务项目
- [ ] 统一配置管理系统
- [ ] 提供 Web 界面
- [ ] 支持更多平台
- [ ] 完善监控和警告机制

## 🤝 贡献指南

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情

## 🙏 致谢

- 感谢 Linux 操作系统提供的强大功能
- 感谢 MySQL 提供的可靠数据存储
- 感谢所有测试和反馈的朋友们

---

<p align="center">
  Made with ❤️ by Linux Developer
</p>
