#include "common.h"
#include <sys/socket.h>  // Socket相关头文件（Linux专用）
#include <netinet/in.h>  // 网络地址结构头文件

// 服务器端口号（可改，比如8888、9999，只要不被占用）
#define SERVER_PORT 8888

std::vector<OnlineUser> g_online_users;  // 在线用户列表
pthread_mutex_t g_mutex;                 // 互斥锁

int main() {
    // -------------------------- 步骤1：初始化互斥锁（保护在线用户列表） --------------------------
    if (pthread_mutex_init(&g_mutex, NULL) != 0) {
        perror("mutex init failed");  // 初始化失败提示
        return 1;
    }

    // -------------------------- 步骤2：创建Socket（相当于“电话”） --------------------------
    // AF_INET：IPv4协议；SOCK_STREAM：TCP协议（可靠传输）；0：默认协议
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("create socket failed");
        pthread_mutex_destroy(&g_mutex);  // 销毁互斥锁
        return 1;
    }
    printf("服务器Socket创建成功！\n");

    // -------------------------- 步骤3：设置Socket选项（允许端口复用，新手不用管） --------------------------
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // -------------------------- 步骤4：绑定端口（给“电话”绑定号码） --------------------------
    struct sockaddr_in server_addr;  // 服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));  // 清空结构
    server_addr.sin_family = AF_INET;              // IPv4
    server_addr.sin_port = htons(SERVER_PORT);     // 端口号（htons：转换为网络字节序）
    server_addr.sin_addr.s_addr = INADDR_ANY;      // 绑定所有网卡（允许任何IP连接）

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind port failed");
        close(server_sock);
        pthread_mutex_destroy(&g_mutex);
        return 1;
    }
    printf("服务器绑定端口 %d 成功！\n", SERVER_PORT);

    // -------------------------- 步骤5：监听连接（开启“电话铃声”，等待客户端打过来） --------------------------
    // 第二个参数：等待连接的队列长度（最多同时有10个客户端等待连接）
    if (listen(server_sock, 10) == -1) {
        perror("listen failed");
        close(server_sock);
        pthread_mutex_destroy(&g_mutex);
        return 1;
    }
    printf("服务器开始监听，等待客户端连接...（端口：%d）\n", SERVER_PORT);

    // -------------------------- 步骤6：循环接收客户端连接（一直等“电话”） --------------------------
    struct sockaddr_in client_addr;  // 客户端地址结构
    socklen_t client_addr_len = sizeof(client_addr);  // 地址长度

    while (true) {
        // accept()：阻塞等待客户端连接，返回“与客户端通信的socket”（相当于“专线电话”）
        int* client_sock = new int;  // 用指针存，方便传给线程（新手不用管）
        *client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (*client_sock == -1) {
            perror("accept client failed");
            delete client_sock;
            continue;
        }
        printf("新客户端连接成功！（socket：%d）\n", *client_sock);

        // -------------------------- 步骤7：创建线程，单独处理这个客户端 --------------------------
        pthread_t tid;  // 线程ID
        // pthread_create：创建线程，参数依次是：线程ID、线程属性（NULL默认）、线程函数、传给函数的参数
        if (pthread_create(&tid, NULL, handle_client, (void*)client_sock) != 0) {
            perror("create thread failed");
            close(*client_sock);
            delete client_sock;
            continue;
        }

        // 分离线程：线程结束后自动释放资源（新手不用管，避免内存泄漏）
        pthread_detach(tid);
    }

    // -------------------------- 步骤8：关闭服务器（理论上不会执行到这里，因为上面是死循环） --------------------------
    close(server_sock);
    pthread_mutex_destroy(&g_mutex);
    return 0;
}