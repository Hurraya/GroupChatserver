#include<iostream>
#include<string>
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#ifdef _WIN32
    #include<winsock2.h>
    #pragma comment(lib, "ws2_32.lib")  // 编译完记得把ws_32.lib 静态库链接进来

#else  
    #include <sys/socket.h>
    #include<netinet/in.h>
    #include<unistd.h>
#endif

int main(){
    SetConsoleOutputCP(CP_UTF8); // 设置控制台输出为 UTF-8
#ifdef _WIN32
    // windows 专属的网络初始化
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    // 1.创建socket(欢迎socket)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1){
        std::cerr << "创建 socket 失败\n";
        return 1;
    }

    // 2.绑定端口（bind）(设定sockaddr)
    sockaddr_in address;
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // 监听所有的网卡
    address.sin_port = htons(8080);         // 端口8080

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1){

        std::cerr << "端口绑定失败\n";
        return 1;
    }

    // 3.监听
    listen(server_fd, 3);   // 3个排队队列
    std::cout << "服务器已启动, 正在监听 8080端口...\n";

    // 4.等待连接
    std::cout << "等待客户端连接中...\n";
    int client_socket = accept(server_fd, nullptr, nullptr);

    // 5.接收数据并回复
    char buffer[1024] = {0};
    recv(client_socket, buffer, sizeof(buffer), 0);
    std::cout << "收到客户端消息：" << buffer << "\n";

    std::string reply = "Hello, client! I am server";
    send(client_socket, reply.c_str(), reply.length(), 0);

#ifdef _WIN32
    closesocket(client_socket);
    closesocket(server_fd);
    WSACleanup();
#else  
    close(client_socket);
    close(server_fd);
#endif

    return 0;

}