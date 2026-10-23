#include<iostream>
#include<string>
#include<sstream>
#include<thread>
#ifdef _WIN32
    #include<winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include<sys/socket.h>
    #include<netinet/in.h>
    #include<unistd.h>
#endif

// 处理客户端(把recv也包装进去)
void handleClient(int client_socket){

    char buffer[4096] = {0};
    
    // 接收浏览器的数据（阻塞）
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0){

        std::string request(buffer);

        if (request.find("favicon.ico") == std::string::npos){

            // 1.判断是 GET 还是 POST
            if (request.find("GET / ") == 0){
                // 返回带有表单的HTML文件
                std::string html = 
                "<html><head><meta charset='utf-8'></head><body>"
                "<h1 style='color:blue;'>Welcom to c++ Server</h1>"
                "<form action='/submit' method='POST'>"
                " <label>please input your name:</label>"
                " <input type='text' name='username' required>"
                " <button type='submit'> submit to C++ server</button>"
                "</form>"
                "</body></html>";

                std::string response =  "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Content-Length: " + std::to_string(html.length()) + "\r\n"
                "Connection: close\r\n\r\n" + html;

                send(client_socket, response.c_str(), response.length(),0);

            }else if (request.find("POST /submit" ) == 0){

                // 解析post的请求 Body（例如username=张三）
                size_t pos = request.find("\r\n\r\n");  // 这是body部分前的字符串
                if (pos != std::string::npos){
                    std::string body  = request.substr(pos + 4);
                    std::cout << "[processe " << std::this_thread::get_id() <<"] form: " << body << std::endl;
                }

                // 返回一个成功画面
                std::string html = "<html><head><meta charset='utf-8'></head><body><h1>successfull submit!!please look at console</h1></body></html>";
                std::string response =  "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Content-Length: " + std::to_string(html.length()) + "\r\n"
                "connection: close\r\n\r\n" + html;
                send(client_socket, response.c_str(), response.length(),0);

            }
            else{ // 找不到页面
            std::string response =  "HTTP/1.1 404 Not Found\r\n\r\n";
            send(client_socket, response.c_str(), response.length(),0);
        }

           
    }
    }
// 处理完毕，关闭该客户端的连接
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
    
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
    std::cout << " multithreading web server start working, please visit http://localhost:8080 in browser\n";

    while(true){
        std::cout << "waitting to connect new client...\n";
        int client_socket = accept(server_fd, nullptr, nullptr);  // blocking
        std::cout << "recieved new client, giving socket [" << client_socket << "]. start subthread to hanfdle\n";

       // 把具体的处理任务丢给子线程，然后立刻让他自己飞（detach）
       std::thread worker(handleClient, client_socket);
        worker.detach();

    }
    return 0;
}