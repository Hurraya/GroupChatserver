#include<iostream>
#include<string>
#ifdef _WIN32
    #include<winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include<sys/socket.h>
    #include<netinet/in.h>
    #include<unistd.h>
#endif

int main(){
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), & wsaData);
#endif

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address))){
        std::cout << "failed to bind\n";
        return 1;
    }
    listen(server_fd, 10);
    std::cout << " web server start working, please visit http://localhost:8080 in browser\n";

    while(true){
        int client_socket = accept(server_fd, nullptr, nullptr);  // blocking

        // 1.接受浏览器的请求报文（随便读一下）
        char buffer[2048] = {0};
        recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        std::cout << "recieve the request from browser: \n" << buffer << "\n";

        // 2.构造一个符合的HTTP协议的响应
        std::string html = "<html><body><h1 style='color:red;'>Hello from C++ Server!</h1></body></html>";

        std::string response = 
        "HTTP/1.1 200 OK\r\n"   // 状态行
        "Content-Type: text/html; charset=utf-8\r\n"   // 告诉浏览器这是网页，且编码是UTF-8
        "Content-Length: " + std::to_string(html.length()) + "\r\n" // 正文宽度、
        "Connection :close\r\n"                 // 告诉浏览器发完就断开连接
        "\r\n" +                           // 一个空行，代表Header结束
        html;
        
        // 3.发送响应给浏览器
        send(client_socket, response.c_str(), response.length(), 0);

        // 4.关闭这次连接
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif

    }
    return 0;
}