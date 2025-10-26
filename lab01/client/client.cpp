#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // 链接Winsock库

const int PORT = 8888;       // 与VMware端口转发的「主机端口」一致
const int BUFFER_SIZE = 1024;
const std::string EXIT_CMD = "exit";

void error_exit(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
    WSACleanup();
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "用法：" << argv[0] << " <虚拟机IP>" << std::endl;
        return EXIT_FAILURE;
    }
    std::string server_ip = argv[1];

    // 初始化Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error_exit("WSAStartup 失败");
    }

    // 创建Socket
    SOCKET client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == INVALID_SOCKET) {
        error_exit("Socket 创建失败");
    }

    // 配置服务器地址
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        error_exit("无效的服务器 IP");
    }

    // 连接服务器
    std::cout << "正在连接虚拟机服务端 " << server_ip << ":" << PORT << " ..." << std::endl;
    if (connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        error_exit("连接失败，请检查：1.虚拟机是否启动 2.IP和端口是否正确 3.防火墙是否拦截");
    }
    std::cout << "连接成功！" << std::endl;

    // 双向通信
    char buffer[BUFFER_SIZE] = {0};
    while (true) {
        std::cout << "客户端：";
        std::string client_msg;
        std::getline(std::cin, client_msg);

        send(client_fd, client_msg.c_str(), client_msg.size(), 0);

        if (client_msg == EXIT_CMD) {
            std::cout << "客户端退出" << std::endl;
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        int recv_len = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (recv_len <= 0) {
            std::cout << "服务端断开连接" << std::endl;
            break;
        }

        std::cout << "服务端：" << buffer << std::endl;
    }

    closesocket(client_fd);
    WSACleanup();
    return 0;
}