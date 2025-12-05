// client_gui.cpp - 运行在 Windows
#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <windows.h>
#include <commdlg.h> // Windows 文件对话框库
#include "protocol.h"

#pragma comment(lib, "ws2_32.lib")  // 链接 Winsock 库
#pragma comment(lib, "comdlg32.lib") // 链接通用对话框库

using namespace std;

// 封装 Windows 文件选择 GUI
string open_file_dialog() {
    OPENFILENAMEA ofn;       // 公共对话框结构
    char szFile[260] = {0};  // 缓冲区

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return string(ofn.lpstrFile);
    }
    return "";
}

// 从路径中提取纯文件名 (处理 Windows 的反斜杠)
string get_filename(const string& path) {
    size_t pos = path.find_last_of("\\/");
    if (pos != string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

int main() {
    // 1. 初始化 Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        return 1;
    }

    // 2. 输入服务端 IP
    string server_ip;
    cout << "Enter Linux Server IP: ";
    cin >> server_ip;

    // 3. 弹出 GUI 选择文件
    cout << "Opening file selector..." << endl;
    string filepath = open_file_dialog(); // 核心 UI 功能
    if (filepath.empty()) {
        cout << "No file selected. Exiting." << endl;
        WSACleanup();
        return 0;
    }
    cout << "Selected File: " << filepath << endl;

    // 4. 读取文件信息
    ifstream infile(filepath, ios::binary | ios::ate);
    if (!infile.is_open()) {
        cerr << "Error opening file!" << endl;
        WSACleanup();
        return 1;
    }
    uint64_t filesize = infile.tellg();
    infile.seekg(0, ios::beg);

    // 5. 连接服务器
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Connection failed!" << endl;
        infile.close();
        WSACleanup();
        return 1;
    }

    // 6. 发送协议头
    FileHeader header;
    string filename = get_filename(filepath);
    strncpy(header.filename, filename.c_str(), MAX_FILENAME - 1);
    header.filesize = filesize;

    send(sock, (char*)&header, sizeof(header), 0);

    // 7. 发送文件内容
    char buffer[BUFFER_SIZE];
    cout << "Sending data..." << endl;
    while (!infile.eof()) {
        infile.read(buffer, BUFFER_SIZE);
        int bytes_read = infile.gcount();
        if (bytes_read > 0) {
            send(sock, buffer, bytes_read, 0);
        }
    }

    cout << "File transfer completed." << endl;

    // 清理
    infile.close();
    closesocket(sock);
    WSACleanup();
    system("pause"); // 防止窗口直接关闭
    return 0;
}