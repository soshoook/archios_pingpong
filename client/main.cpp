#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

int main(){
    std::cout << "client started" << "\n";

    // create socket
    int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (file_descriptor == -1){
        std::cerr << "socket: " << std::strerror(errno) << "\n";
        return 1;
    }
    std::cout << "socket created, file descriptor = " << file_descriptor << "\n";
    
    // connect
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr); // presentation to network
    if (connect(file_descriptor, (sockaddr*)&addr, sizeof(addr)) == -1){
        std::cerr << "connect: " << strerror(errno) << "\n";
        close(file_descriptor);
        return 1;
    }
    std::cout << "connected\n";

    // send 
    const char* request = "ping\n";
    ssize_t sent = send(file_descriptor, request, std::strlen(request), MSG_NOSIGNAL); //MSG_NOSIGNAL для возвращения понятной ошибки
    if (sent == -1){
        std::cerr << "send: " << std::strerror(errno) << "\n";
        close(file_descriptor);
        return 1;
    }
    std::cout << "sent: " << request;

    // recv
    char bufer[64];
    ssize_t rec = recv(file_descriptor, bufer, sizeof(bufer), 0);
    if (rec == -1){
        std::cerr << "recv: " << std::strerror(errno) << "\n";
    }
    else if (rec == 0){
        std::cout << "server closed connection\n";
    }
    else {
        std::string msg(bufer, rec);
        std::cout << "received: " << msg;
    }
    // timeout 
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    // set socket option               (socket option, receive timeout)
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1){ 
        std::cerr << "setcokopt: " << std::strerror(errno) << "\n";
        close(file_descriptor);
        return 1;
    } 

    close(file_descriptor);
}