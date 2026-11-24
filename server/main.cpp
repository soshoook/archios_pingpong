#include <iostream>
#include <sys/types.h>    // ssize_t
#include <sys/socket.h>   // socket, bind, listen, accept
#include <netinet/in.h>   // sockaddr_in, htons, INADDR_ANY
#include <unistd.h>       // close
#include <cstring>        // memset, strerror
#include <cerrno>

int main(){
    std::cout << "server started" << "\n";

    // create socket
    int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (file_descriptor == -1){
        std::cerr << "socket: " << strerror(errno) << "\n";
        return 1;
    }
    std::cout << "socket created, file descriptor = " << file_descriptor << "\n";

    // bind
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(5000);
    if (bind(file_descriptor, (sockaddr*)&addr, sizeof(addr)) == -1){
        std::cerr << "bind: " << std::strerror(errno) << "\n";
        close(file_descriptor);
        return 1;
    }
    std::cout << "bind is ok, port 5000\n";

    // listen
    if (listen(file_descriptor, 5) == -1){
        std::cerr << "listen: " << std::strerror(errno) << "\n";
        close(file_descriptor);
        return 1;
    }
    std::cout << "waiting for a client..." << "\n";

    // accept
    int client_fd = accept(file_descriptor, nullptr, nullptr);
    if (client_fd == -1){
        std::cerr << "accept: " << std::strerror(errno) << "\n";
        close(file_descriptor);
        return 1;
    }
    std::cout << "client connected, client_fd = " << client_fd << "\n";

    // recv
    char bufer[64];
    ssize_t n = recv(client_fd, bufer, sizeof(bufer), 0);
    if (n == -1){
        std::cerr << "recv: " << std::strerror(errno) << "\n";
    }
    else if (n == 0) {
        std::cout << "client closed connection\n";
    }
    else {
        std::string msg(bufer, n);
        std::cout << "received: " << msg;
    }

    close(client_fd);
    close(file_descriptor);

    return 0;
}