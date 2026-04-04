# 📡 LinuxNetworkServices

一个基于 Linux 平台的网络服务集合，包含多种实用的网络工具和服务。

## 📁 项目结构

```
LinuxNetworkServices/
├── file-search/          # Linux 磁盘文件远程检索服务
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

| 模块 | 功能 | 技术实现 |
|------|------|----------|
| 文件索引 | 构建和管理文件索引 | 二叉搜索树 |
| 网络通信 | 处理客户端连接 | TCP 多线程 |
| 数据存储 | 持久化存储 | MySQL |
| 结果格式化 | 格式化检索结果 | 自定义格式化 |
| 安全防护 | 路径检查和 SQL 注入防护 | 安全检查机制 |

## 📊 性能指标

| 指标 | 数值 |
|------|------|
| 索引构建时间 | ≤ 30秒（1000个文件） |
| 检索响应时间 | ≤ 1秒（1000个文件） |
| 并发连接数 | 20个 |
| 索引更新间隔 | 5分钟 |
| 记录保留时间 | 30天 |

## 🎯 应用场景

1. **远程文件管理**: 远程服务器文件检索
2. **自动化运维**: 批量文件查找和管理
3. **文件监控**: 实时监控文件变化
4. **数据统计**: 文件热度分析和使用情况统计
5. **开发工具**: 代码文件快速检索

## 🔮 未来计划

- [ ] 添加更多网络服务项目
- [ ] 统一配置管理系统
- [ ] 提供 Web 界面
- [ ] 支持更多平台
- [ ] 完善监控和告警机制

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
