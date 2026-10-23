#include<iostream>
#include<string>
#include<sstream>
#ifdef _WIN32
    #include<winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include<sys/socket.h>
    #include<netinet/in.h>
    #include<unistd.h>
#endif

// 处理HTTP请求并返回响应报文
std::string handleRequest(const std::string& request){
    // 1.判断是 GET 还是 POST
    if (request.find("GET / ") == 0){
        // 返回带有表单的HTML文件
        std::string html = 
        "<html><head><meta charset='utf-8'></head><body>"
        "<h1 style='color:bule;'>Welcom to c++ Server</h1>"
        "<form action='/submit' method='POST'>"
        " <label>please input your name:</label>"
        " <input type='text' name='username' required>"
        " <button type='submit'> submit to C++ server</button>"
        "</form>"
        "</body></html>";

        return "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(html.length()) + "\r\n"
        "connection: close\r\n\r\n" + html;
    }else if (request.find("POST /submit" ) == 0){

        // 解析post的请求 Body（例如username=张三）
        size_t pos = request.find("\r\n\r\n");  // 这是body部分前的字符串
        if (pos != std::string::npos){
            std::string body  = request.substr(pos + 4);
            std::cout << "[data of recived web form]: " << body << std::endl;
        }

        // 返回一个成功画面
        std::string html = "<html><head><meta charset='utf-8'></head><body><h1>successfull submit!!please look at console</h1></body></html>";
        return "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(html.length()) + "\r\n"
        "connection: close\r\n\r\n" + html;
    }

    // 找不到页面
    return "HTTP/1.1 404 Not Found\r\n\r\n";
}

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
        char buffer[4090] = {0};
        recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        // std::cout << "recieve the request from browser: \n" << buffer << "\n";

        std::string request(buffer);

        // 忽略 favicon
        if (request.find("favicon.ico") != std::string::npos){
#ifdef _WIN32
            closesocket(client_socket);
#else
            close(client_socket);
#endif
            continue;

        }

        // 交给handle，拿到响应报文
        std::string response = handleRequest(request);
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