// 防止头文件重复包含（新手不用管，固定写法）
#ifndef COMMON_H
#define COMMON_H

#include <sys/socket.h>   // 包含send()、recv()、socket()等函数声明
#include <netinet/in.h>   // 包含网络地址结构（如struct sockaddr_in）
#include <arpa/inet.h>    // 包含IP地址转换函数（如inet_pton）
#include <unistd.h>       // 包含close()函数声明

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>
#include <string>

// -------------------------- 1. 消息类型（区分不同操作）--------------------------
enum MsgType {
    USER_JOIN,    // 用户加入
    USER_EXIT,    // 用户退出
    GROUP_MSG,    // 群发消息
    PRIVATE_MSG,  // 私聊消息
    SYSTEM_MSG    // 服务器系统公告
};

// -------------------------- 2. 消息结构体（服务器/客户端传递的数据格式）--------------------------
struct Message {
    MsgType type;          // 消息类型（上面的枚举）
    char sender[32];       // 发送者用户名（比如“张三”）
    char receiver[32];     // 接收者用户名（私聊填目标用户，群发填“ALL”）
    char content[1024];    // 消息内容（比如“你好！”）
};

// -------------------------- 3. 在线用户结构体（服务器管理用户用）--------------------------
struct OnlineUser {
    char username[32];  // 用户名
    int sockfd;         // 客户端的socket描述符（相当于“用户的联系方式”）
};

// -------------------------- 4. 全局共享变量（服务器用，需线程安全）--------------------------
extern std::vector<OnlineUser> g_online_users;  // 声明在线用户列表
extern pthread_mutex_t g_mutex;                 // 声明互斥锁

// -------------------------- 5. 函数声明（告诉编译器有这些函数，在其他文件实现）--------------------------
// 服务器线程处理函数（每个客户端对应一个线程）
void* handle_client(void* arg);
// 服务器发送消息给指定客户端（内部用）
void send_msg_to_client(int client_sock, const Message& msg);
// 服务器广播消息（发给所有在线用户，除了发送者）
void broadcast_msg(const Message& msg, int exclude_sock = -1);

#endif  // COMMON_H