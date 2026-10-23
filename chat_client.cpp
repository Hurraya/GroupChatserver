#include <iostream>
#include <string>
#include <thread>
#define WIN32_LEAN_AND_MEAN
#include<windows.h>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif
#include <WS2tcpip.h>

// 接收服务器消息的子线程
void receiveMessages(int sock){
    char buffer[1024];
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);

        if (bytes <= 0){

            std::cout << "与服务器断开 \n";
            break;
        }
        std::cout << buffer << "\n";
    }
    
}

int main(){
    SetConsoleOutputCP(CP_UTF8); // 设置控制台输出为 UTF-8
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        std::cerr << "conncet 失败, 请确保服务器已开启\n";
        return 1;
    }
    std::cout << "已连接到服务器 \n";

    // 开以一个子线程专门听其他客户端的
    std::thread recvThread(receiveMessages, sock);
    recvThread.detach();

    // 主线程负责 发送
    std::string input;
    while ((true))
    {
        std::getline(std::cin, input);
        if (input == "quit") {break;}
        if (!input.empty()){
            send(sock, input.c_str(), input.length(), 0);
        }
    }
    
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    return 0;
}