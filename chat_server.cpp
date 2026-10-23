#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#define WIN32_LEAN_AND_MEAN
#include<windows.h>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
#endif

std::vector<int> clients;    // 存储所有的在线客户端
std::mutex mtx;             // 保护clients的 互斥锁

// 广播给所有人
void broadcastMessage(const std::string& message, int sender_socket){
    // 1.加锁（离开函数时自动解锁）
    std::lock_guard<std::mutex> lock(mtx);

    for(int client : clients){
        // 选择性把消息发送给出自己之外的所有人
        if(client != sender_socket){

            send(client, message.c_str(), message.length(), 0);
        }

    }

}

// 专门服务某个客户端的线程函数
void handleClient(int client_socket){
    char buffer[1024];

    while(true){
        memset(buffer, 0, sizeof(buffer));

        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        // 如果客户端断开连接
        if (bytes_received <= 0){
            std::cout << "[系统] 客户 [" << client_socket << "] 断开connect\n";

            // 删除客户端从clients，加锁
            {
                std::lock_guard<std::mutex> lock(mtx);
                auto it = std::remove(clients.begin(), clients.end(), client_socket);
                clients.erase(it, clients.end());           // remove只是对数据搬运，erase做真正的删除

            }// 锁在这里释放（出作用域就释放）

            #ifdef _WIN32
                closesocket(client_socket);
            #else
                close(client_socket);
            #endif
            break;
        }

        // 收到了信息。构造格式
        std::string msg = "[Client" + std::to_string(client_socket) + "]"  + buffer;
        std::cout << msg << "\n";

        // 广播给其他人
        broadcastMessage(msg, client_socket);
        
    }

}

int main(){
    SetConsoleOutputCP(CP_UTF8); // 设置控制台输出为 UTF-8
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1){

        std::cerr << "绑定失败 端口可能被占用 \n";
        return 1;
    }
    listen(server_fd, 10);
    std::cout << "群聊服务器启,监听 8080 端口...\n";

    while(true){
        std::cout << "等待新客户端连接...\n";
        int client_socket = accept(server_fd, nullptr, nullptr);

        // 把新客户加入列表 （必须加锁）
        {
            std::lock_guard<std::mutex> lock(mtx);
            clients.push_back(client_socket);
        }
        
        std::cout << "[系统] 新客户端 " << client_socket << "已连入!\n";

        // 开个子线程服务这个client
        std::thread t(handleClient, client_socket);
        t.detach();
    }
#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    /* 服务器这里是一个死循环，一般不会终止，除非是按了CTRL+C强制终止。Linux系统会自动回收这个进程的所有资源（包括文件描述符）*/
    close(server_fd);   
#endif 
    return 0;
}

    

    