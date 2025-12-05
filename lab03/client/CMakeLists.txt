cmake_minimum_required(VERSION 3.10)

# 项目名称
project(Lab3_FileTransfer)

# 设置 C++ 标准为 C++11 或更高
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ==========================================
# 平台检测与编译配置
# ==========================================

if (WIN32)
    # --------------------------------------
    # Windows 环境 (编译客户端)
    # --------------------------------------
    message(STATUS "Detected System: Windows - Building Client with GUI")

    # 添加可执行文件：客户端
    add_executable(client client_gui.cpp)

    # 链接 Windows 特有的库
    # ws2_32: Winsock 网络库
    # comdlg32: Windows 文件选择对话框库
    target_link_libraries(client ws2_32 comdlg32)

elseif (UNIX)
    # --------------------------------------
    # Linux/MacOS 环境 (编译服务端)
    # --------------------------------------
    message(STATUS "Detected System: Linux/Unix - Building Server")

    # 添加可执行文件：服务端
    add_executable(server server.cpp)
    
    # Linux 下 Socket 库通常是标准库的一部分，不需要额外链接
    # 如果使用了多线程 (pthread)，需要取消下面这行的注释:
    # target_link_libraries(server pthread)

else()
    message(WARNING "Unknown platform, nothing will be built.")
endif()