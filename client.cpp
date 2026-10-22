#include<iostream>
#include<string>
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#ifdef _WIN32
    #include<winsock2.h>
    #pragma  comment(lib, "ws2_32.lib")
#else
    #include<sys/socket.h>
    #include<arpa/inet.h>
    #include<unistd.h>
#endif
#include <WS2tcpip.h>

int main(){
    SetConsoleOutputCP(CP_UTF8); // 设置控制台输出为 UTF-8
#ifdef _WIN32    // windows网络初始化
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    // 1.创建socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // 2.准备目标地址（服务器的IP 和 port）
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port= htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // 127.0.0.1 代表本机

    // 3.connect
    std::cout << "正在connect服务器...\n";
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        std::cerr << "connect失败,请确保服务器开启\n";
        return 1;
    }
    std::cout << "connect成功\n";

    // 4.发送消息
    std::string msg = "Hello, server! 我是 client.";
    send(sock, msg.c_str(), msg.length(), 0);

    // 5.接收服务器回复（recv）
    char buffer[1024] = {0};
    recv(sock, buffer, sizeof(buffer), 0);
    std::cout << "收到服务器回复：" << buffer << "\n";

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

}