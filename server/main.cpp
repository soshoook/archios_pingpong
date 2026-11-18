#include <iostream>
#include <sys/socket.h>   // socket, bind, listen, accept
#include <netinet/in.h>   // sockaddr_in, htons, INADDR_ANY
#include <unistd.h>       // close
#include <cstring>        // memset, strerror
#include <cerrno>

int main(){
    std::cout << "server started" << "\n";

    int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (file_descriptor == -1){
        std::cerr << "socket: " << strerror(errno) << "\n";
        return 1;
    }
    std::cout << "socket created, file descriptor = " << file_descriptor << "\n";

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(5000);
    if (bind(file_descriptor, (sockaddr*)&addr, sizeof(addr)) == -1){
        std::cerr << "bind: " << std::strerror << "\n";
        close(file_descriptor);
        return 1;
    }
    std::cout << "bind is ok,, port 5000\n";
    close(file_descriptor);

    return 0;
}