#include <iostream>
#include <sys/types.h>    // ssize_t
#include <sys/socket.h>   // socket, bind, listen, accept
#include <netinet/in.h>   // sockaddr_in, htons, INADDR_ANY
#include <unistd.h>       // close
#include <cstring>        // memset, strerror
#include <cerrno>

enum class State {WaitingRequest, HandleRequest, SendingReply, Error, Done};

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

    

    char bufer[64];
    while (true){
        int client_fd = accept(file_descriptor, nullptr, nullptr);
        if (client_fd == -1){
            std::cerr << "accept: " << std::strerror(errno) << "\n";
            continue;
        }
        // timeout 
        timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        // set socket option               (socket option, receive timeout)
        if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1){ 
            std::cerr << "setcokopt: " << std::strerror(errno) << "\n";
            close(client_fd);
            continue;
        }
        
        State state = State::WaitingRequest;
        ssize_t rec = 0;
        std::string msg;
        std::string error_text;

        while (state != State::Done) {
            switch (state){
                case State::WaitingRequest: {
                    rec = recv(client_fd, bufer, sizeof(bufer), 0);
                    if (rec > 0){
                        msg = std::string(bufer, rec);
                        state = State::HandleRequest;
                    }
                    else if (rec == 0){
                        error_text = "client closed connection";
                        state = State::Error;
                    }
                    else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK){
                            error_text = "timeout";
                        }
                        else {
                            error_text = std::strerror(errno);
                        }
                        state = State::Error;
                    }
                    break;
                }
                case State::HandleRequest: {
                    if (msg == "ping\n"){
                        std::cout << "received: " << msg;
                        state = State::SendingReply;
                    }
                    else {
                        error_text = "unexpected message";
                        state = State::Error;
                    }
                    break;
                }
                case State::SendingReply: {
                    ssize_t sent = send(client_fd, "pong\n", 5, MSG_NOSIGNAL);
                    if (sent == -1){
                        error_text = std::strerror(errno);
                        state = State::Error;
                    }
                    else {
                        std::cout << "sent: " << "pong\n";
                        state = State::Done;
                    }
                    break;
                }
                case State::Error: {
                    std::cerr << "client error: " << error_text << "\n";
                    state = State::Done;
                    break;
                }
                default:
                    break;
            }
        }
        close(client_fd);
    }

}