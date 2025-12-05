// server.cpp - 运行在 Ubuntu
#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "protocol.h"

using namespace std;

void handle_client(int client_sock) {
    FileHeader header;
    
    // 1. 接收协议头
    int len = recv(client_sock, &header, sizeof(header), 0);
    if (len <= 0) {
        close(client_sock);
        return;
    }

    cout << "[Server] Receiving file: " << header.filename 
         << " (" << header.filesize << " bytes)" << endl;

    // 2. 准备写入文件 (以二进制模式)
    ofstream outfile(header.filename, ios::binary);
    if (!outfile.is_open()) {
        cerr << "[Error] Cannot create file!" << endl;
        close(client_sock);
        return;
    }

    // 3. 循环接收文件内容
    char buffer[BUFFER_SIZE];
    uint64_t total_received = 0;
    while (total_received < header.filesize) {
        int bytes_read = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) break;
        
        outfile.write(buffer, bytes_read);
        total_received += bytes_read;
    }

    outfile.close();
    cout << "[Server] File saved successfully." << endl;
    close(client_sock);
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 绑定与监听
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }
    listen(server_sock, 5);

    cout << "[Server] Listening on port " << PORT << "..." << endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_sock >= 0) {
            cout << "[Server] Client connected: " << inet_ntoa(client_addr.sin_addr) << endl;
            handle_client(client_sock);
        }
    }
    
    close(server_sock);
    return 0;
}