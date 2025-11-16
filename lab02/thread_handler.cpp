#include "common.h"

// -------------------------- 辅助函数：发送消息给单个客户端 --------------------------
void send_msg_to_client(int client_sock, const Message& msg) {
    // send()：把msg的数据发给client_sock对应的客户端
    // sizeof(msg)：消息的总长度（保证接收完整）
    ssize_t ret = send(client_sock, &msg, sizeof(msg), 0);
    if (ret == -1) {
        perror("send to client failed");  // 出错提示（新手不用管，调试用）
    }
}

// -------------------------- 辅助函数：广播消息给所有在线用户（可排除发送者） --------------------------
void broadcast_msg(const Message& msg, int exclude_sock) {
    // 加互斥锁：操作在线用户列表前必须锁，防止多线程同时改列表
    pthread_mutex_lock(&g_mutex);
    
    // 遍历所有在线用户
    for (const auto& user : g_online_users) {
        // 排除指定的sock（比如群发时不发给自己）
        if (user.sockfd != exclude_sock) {
            send_msg_to_client(user.sockfd, msg);
        }
    }
    
    // 解锁：操作完列表必须解锁，让其他线程能用
    pthread_mutex_unlock(&g_mutex);
}

// -------------------------- 核心：每个客户端的线程处理函数 --------------------------
void* handle_client(void* arg) {
    // arg是客户端的socket描述符（从server.cpp传过来）
    int client_sock = *(int*)arg;
    delete (int*)arg;  // 释放内存（新手不用管，防止内存泄漏）
    
    Message msg;
    OnlineUser new_user;
    new_user.sockfd = client_sock;  // 记录当前客户端的“联系方式”

    // -------------------------- 步骤1：接收客户端的“用户名”（用户加入） --------------------------
    ssize_t recv_ret = recv(client_sock, &msg, sizeof(msg), 0);
    if (recv_ret <= 0) {  // 接收失败或客户端断开
        close(client_sock);
        pthread_exit(NULL);  // 退出线程
    }
    // 保存用户名到在线用户结构体
    strncpy(new_user.username, msg.sender, sizeof(new_user.username)-1);

    // -------------------------- 步骤2：广播“用户加入”消息给所有人 --------------------------
    msg.type = USER_JOIN;
    snprintf(msg.content, sizeof(msg.content)-1, 
             "用户【%s】加入群聊！当前在线人数：%d", 
             new_user.username, (int)g_online_users.size() + 1);  // 人数+1（新用户还没加列表）
    // 加锁→把新用户加入列表→解锁
    pthread_mutex_lock(&g_mutex);
    g_online_users.push_back(new_user);
    pthread_mutex_unlock(&g_mutex);
    // 广播加入消息（所有人都能看到）
    broadcast_msg(msg);
    printf("用户【%s】上线，当前在线：%d人\n", new_user.username, (int)g_online_users.size());

    // -------------------------- 步骤3：循环接收客户端的消息（核心循环） --------------------------
    while (true) {
        recv_ret = recv(client_sock, &msg, sizeof(msg), 0);
        if (recv_ret <= 0) {  // 客户端断开（比如关闭终端）
            break;
        }

        // 根据消息类型处理不同逻辑
        switch (msg.type) {
            // -------------------------- 情况1：用户主动退出 --------------------------
            case USER_EXIT: {
                // 加锁→从在线列表删除当前用户→解锁
                pthread_mutex_lock(&g_mutex);
                for (auto it = g_online_users.begin(); it != g_online_users.end(); ++it) {
                    if (it->sockfd == client_sock) {
                        g_online_users.erase(it);  // 删除用户
                        break;
                    }
                }
                pthread_mutex_unlock(&g_mutex);

                // 广播“用户退出”消息
                snprintf(msg.content, sizeof(msg.content)-1, 
                         "用户【%s】退出群聊！当前在线人数：%d", 
                         new_user.username, (int)g_online_users.size());
                broadcast_msg(msg);
                printf("用户【%s】下线，当前在线：%d人\n", new_user.username, (int)g_online_users.size());
                goto exit_thread;  // 跳转到退出线程（新手不用管，相当于break所有循环）
            }

            // -------------------------- 情况2：群发消息 --------------------------
            case GROUP_MSG: {
                // 拼接群发消息内容（加上发送者）
                char group_content[1024];
                snprintf(group_content, sizeof(group_content)-1, 
                         "[群发] %s：%s", msg.sender, msg.content);
                strncpy(msg.content, group_content, sizeof(msg.content)-1);
                // 广播给所有人（排除发送者自己，避免自己看自己的消息）
                broadcast_msg(msg, client_sock);
                break;
            }

            // -------------------------- 情况3：私聊消息 --------------------------
            case PRIVATE_MSG: {
                // 加锁→遍历在线用户，找接收者→解锁
                pthread_mutex_lock(&g_mutex);
                int target_sock = -1;
                for (const auto& user : g_online_users) {
                    if (strcmp(user.username, msg.receiver) == 0) {  // 找到目标用户名
                        target_sock = user.sockfd;
                        break;
                    }
                }
                pthread_mutex_unlock(&g_mutex);

                // 拼接私聊消息内容
                char private_content[1024];
                if (target_sock != -1) {  // 找到接收者
                    snprintf(private_content, sizeof(private_content)-1, 
                             "[私聊] %s：%s", msg.sender, msg.content);
                    strncpy(msg.content, private_content, sizeof(msg.content)-1);
                    send_msg_to_client(target_sock, msg);  // 只发给接收者
                } else {  // 没找到接收者（比如用户不在线）
                    snprintf(private_content, sizeof(private_content)-1, 
                             "[系统提示] 用户【%s】不在线或不存在！", msg.receiver);
                    strncpy(msg.content, private_content, sizeof(msg.content)-1);
                    send_msg_to_client(client_sock, msg);  // 提示发送者
                }
                break;
            }

            // -------------------------- 情况4：服务器系统公告（客户端发这个类型无效） --------------------------
            default:
                break;
        }
    }

// -------------------------- 步骤4：退出线程（释放资源） --------------------------
exit_thread:
    close(client_sock);  // 关闭客户端socket
    pthread_exit(NULL);  // 退出线程
}