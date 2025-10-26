#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#define _GNU_SOURCE
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int PORT = 8888;               // 与端口转发的虚拟机端口一致
const int BUFFER_SIZE = 1024;
const std::string EXIT_CMD = "exit";

class SocketUtils {
public:
    static void error_exit(const std::string& msg) {
        std::cerr << "[ERROR] " << msg << ": " << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    static std::string addr_to_string(const sockaddr_in& addr) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
        return std::string(ip) + ":" + std::to_string(ntohs(addr.sin_port));
    }
};

#endif // SOCKET_UTILS_H