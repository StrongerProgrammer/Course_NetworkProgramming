// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>

// 定义文件名最大长度
#define MAX_FILENAME 256
// 定义缓冲区大小 (4KB)
#define BUFFER_SIZE 4096
// 服务端监听端口
#define PORT 8888

// 传输协议头：先发这个结构体，再发文件内容
struct FileHeader {
    char filename[MAX_FILENAME]; // 文件名
    uint64_t filesize;           // 文件大小(字节)
};

#endif