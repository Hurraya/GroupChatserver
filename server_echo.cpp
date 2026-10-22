#include<iostream>
#include<string>
#include<vector>
#include<thread>
#define WIN32_LEAN_AND_MEAN
#include<windows.h>

#ifdef _WIN32
    #include<winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include<sys/socket.h>
    #include<netinet/in.h>
    #include<unistd.h>
#endif

// 这个函数专门负责和某一个特定的客户端通信
void handleClient(int client_socket){
    char buffer[1024];

    while(true){

        // 清空缓存区
        memset(buffer, 0, sizeof(buffer));
        // 阻塞等待接收数据
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) -1,0);

        // 如果recv返回0，或小于0，说明用户断开了连接
        if (bytes_received <= 0){
            std::cout << "[系统] 一个用户断开了connect \n";
            break;
        }

        std::cout << "[收到]" << buffer << "\n";

        // 把消息加上 [Echo] 原样发回去
        std::string reply = "[Echo] ";
        reply += buffer;
        send(client_socket, reply.c_str(), reply.length(), 0);

    }

    #ifdef _WIN32
        closesocket(client_socket);
    #else
        close(client_socket);
    #endif


}

int main(){
    SetConsoleOutputCP(CP_UTF8); // 设置控制台输出为 UTF-8
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in  address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // 监听所有网卡
    address.sin_port = htons(8080);  

    // 绑定端口，如果报错，可能是8080被占用了
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1){
        std::cerr << "绑定失败 可能被占用\n";
        return 1;
    }

    listen(server_fd, 10);
    while (true)
    {
        std::cout << "等待客户connect...\n";
        // 主线程卡在这里
        int client_socket = accept(server_fd, nullptr, nullptr);
        std::cout << "有新的客户端连入! FD = " << client_socket << "\n";

        // 启动一个子线程，专门服务这个客户端
        std::thread t(handleClient, client_socket);
        t.detach();     // detach 是让子线程在后台跑，主线程不管它了
    }
    
#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#endif
    return 0;

}