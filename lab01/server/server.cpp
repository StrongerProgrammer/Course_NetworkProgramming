#include "socket_utils.h"

int main() {
    int server_fd, client_fd;
    sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};

    // 创建 Socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        SocketUtils::error_exit("Socket 创建失败");
    }

    // 允许端口复用
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        SocketUtils::error_exit("setsockopt 失败");
    }

    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 绑定端口
    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        SocketUtils::error_exit("端口绑定失败");
    }

    // 监听连接
    if (listen(server_fd, 5) < 0) {
        SocketUtils::error_exit("监听失败");
    }
    std::cout << "服务端启动，监听端口 " << PORT << " ..." << std::endl;

    // 接受客户端连接
    if ((client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_addr_len)) < 0) {
        SocketUtils::error_exit("接受连接失败");
    }
    std::cout << "客户端连接：" << SocketUtils::addr_to_string(client_addr) << std::endl;

    // 双向通信
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t recv_len = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (recv_len < 0) {
            SocketUtils::error_exit("接收消息失败");
        } else if (recv_len == 0) {
            std::cout << "客户端断开连接" << std::endl;
            break;
        }

        std::string client_msg(buffer);
        std::cout << "客户端：" << client_msg << std::endl;

        if (client_msg == EXIT_CMD) {
            std::cout << "服务端即将退出" << std::endl;
            break;
        }

        std::cout << "服务端：";
        std::string server_msg;
        std::getline(std::cin, server_msg);

        if (server_msg == EXIT_CMD) {
            send(client_fd, server_msg.c_str(), server_msg.size(), 0);
            std::cout << "服务端退出" << std::endl;
            break;
        }

        if (send(client_fd, server_msg.c_str(), server_msg.size(), 0) < 0) {
            SocketUtils::error_exit("发送消息失败");
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}