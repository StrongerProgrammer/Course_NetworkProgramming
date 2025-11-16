# 编译器（固定）
CC = g++
# 编译选项：-Wall显示警告，-g调试用，-pthread链接线程库（必须加！）
CFLAGS = -Wall -g -pthread
# 目标文件（要生成的可执行文件）
TARGETS = server client

# 默认目标（执行make时自动执行）
all: $(TARGETS)

# 编译服务器：server.cpp + thread_handler.cpp → 生成server可执行文件
server: server.cpp thread_handler.cpp common.h
	$(CC) $(CFLAGS) -o server server.cpp thread_handler.cpp

# 编译客户端：client.cpp → 生成client可执行文件
client: client.cpp common.h
	$(CC) $(CFLAGS) -o client client.cpp

# 清理编译产物（执行make clean时用）
clean:
	rm -f $(TARGETS)  # 删除server和client可执行文件