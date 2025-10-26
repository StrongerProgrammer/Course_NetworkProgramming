# 网络程序设计实验lab01

## 项目介绍

本项目实现了 **Windows 客户端** 与 **Linux 服务端** 之间的 TCP 跨平台通信，基于 Socket 编程模型，展示了 Linux 系统 Socket API 与 Windows Winsock API 的差异与适配。

## 环境要求

### 服务端（Linux 虚拟机）

- 操作系统：Ubuntu22.04
- 编译工具：`g++`、`make`
-  ssh server服务器
- 依赖：系统 Socket 库（无需额外安装）

### 客户端（Windows 主机）

- 操作系统：Windows 11
- 开发工具：VSCode
- 编译工具：MinGW（需添加 `bin` 目录到系统环境变量）
- 依赖：Winsock2 库（系统内置，编译时需链接 `ws2_32.lib`）

## linux服务端启动步骤

```bash
make #生成server可执行文件

./server #启动ubuntu服务端
```

## windows客户端启动步骤

```bash
chcp 65001 #切换终端为utf-8编码，不然会出现乱码问题

g++ client.cpp -o client.exe -lws2_32 #生成 client.exe。

client.exe 192.168.100.128 #请求连接服务端
```

