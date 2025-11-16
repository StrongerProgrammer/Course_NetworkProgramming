#include "common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // 用于IP地址转换

// 客户端接收消息的线程函数（单独线程，避免输入时阻塞）
void* recv_msg_thread(void* arg) {
    int client_sock = *(int*)arg;
    Message msg;

    while (true) {
        // 接收服务器发来的消息
        ssize_t ret = recv(client_sock, &msg, sizeof(msg), 0);
        if (ret <= 0) {  // 服务器断开或接收失败
            printf("\n服务器断开连接！按任意键退出...\n");
            exit(0);  // 退出客户端
        }

        // 根据消息类型显示不同内容（不用处理USER_JOIN/USER_EXIT，服务器已拼好内容）
        printf("\n%s\n", msg.content);  // 直接打印服务器发来的内容
        printf("请输入消息（群发直接输，私聊格式：@用户名 内容，退出输/quit）：");
        fflush(stdout);  // 刷新输出（防止提示语不显示）
    }
}

int main(int argc, char* argv[]) {
    // 检查命令行参数：客户端启动需传“服务器IP”（比如./client 192.168.1.100）
    if (argc != 2) {
        printf("用法：./client 服务器IP地址\n");
        printf("例子：./client 192.168.1.100 （服务器在本地则用127.0.0.1）\n");
        return 1;
    }

    // -------------------------- 步骤1：创建客户端Socket --------------------------
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1) {
        perror("create client socket failed");
        return 1;
    }

    // -------------------------- 步骤2：连接服务器 --------------------------
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);  // 服务器端口（必须和server.cpp一致）
    // 将字符串IP转换为网络字节序（比如“192.168.1.100”→二进制）
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        printf("无效的服务器IP地址！\n");
        close(client_sock);
        return 1;
    }

    // 连接服务器（相当于“打给服务器”）
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect to server failed");
        close(client_sock);
        return 1;
    }
    printf("连接服务器成功！\n");

    // -------------------------- 步骤3：输入用户名并发送“加入”消息 --------------------------
    Message msg;
    char username[32];
    printf("请输入你的用户名：");
    fgets(username, sizeof(username)-1, stdin);
    // 去掉fgets读入的换行符（比如输入“张三\n”→变成“张三”）
    username[strcspn(username, "\n")] = '\0';
    strncpy(msg.sender, username, sizeof(msg.sender)-1);
    msg.type = USER_JOIN;  // 消息类型：用户加入
    send(client_sock, &msg, sizeof(msg), 0);  // 发给服务器

    // -------------------------- 步骤4：创建“接收消息线程”（单独收消息，不影响输入） --------------------------
    pthread_t tid;
    if (pthread_create(&tid, NULL, recv_msg_thread, (void*)&client_sock) != 0) {
        perror("create recv thread failed");
        close(client_sock);
        return 1;
    }
    pthread_detach(tid);  // 分离线程

    // -------------------------- 步骤5：循环读取用户输入（发送消息） --------------------------
    char input[1024];
    while (true) {
        printf("请输入消息（群发直接输，私聊格式：@用户名 内容，退出输/quit）：");
        fgets(input, sizeof(input)-1, stdin);
        input[strcspn(input, "\n")] = '\0';  // 去掉换行符

        // -------------------------- 情况1：用户退出 --------------------------
        if (strcmp(input, "/quit") == 0) {
            msg.type = USER_EXIT;
            send(client_sock, &msg, sizeof(msg), 0);
            printf("正在退出...\n");
            sleep(1);  // 等服务器广播退出消息
            break;
        }

        // -------------------------- 情况2：私聊消息（格式：@用户名 内容） --------------------------
        if (input[0] == '@') {
            // 拆分“@用户名 内容”→提取用户名和内容
            char* space_pos = strchr(input + 1, ' ');  // 找第一个空格（跳过@）
            if (space_pos == NULL) {  // 格式错误（比如“@张三”没加内容）
                printf("私聊格式错误！正确格式：@用户名 内容（比如@李四 你好）\n");
                continue;
            }
            // 提取接收者用户名（@到空格之间）
            int receiver_len = space_pos - (input + 1);
            strncpy(msg.receiver, input + 1, receiver_len);
            msg.receiver[receiver_len] = '\0';  // 手动加结束符
            // 提取消息内容（空格之后）
            strncpy(msg.content, space_pos + 1, sizeof(msg.content)-1);
            msg.type = PRIVATE_MSG;  // 消息类型：私聊
        }

        // -------------------------- 情况3：群发消息（默认） --------------------------
        else {
            strncpy(msg.content, input, sizeof(msg.content)-1);
            strcpy(msg.receiver, "ALL");  // 群发接收者填“ALL”
            msg.type = GROUP_MSG;  // 消息类型：群发
        }

        // 发送消息给服务器
        send(client_sock, &msg, sizeof(msg), 0);
    }

    // -------------------------- 步骤6：关闭客户端 --------------------------
    close(client_sock);
    return 0;
}