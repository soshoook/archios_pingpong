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

    enum class State {Compose, WaitingForReply, ReadingReply, HandleError, Done};
    State state = State::Compose;

    const int max_attempts = 3;
    int attempt = 0;
    int code_exit = 0; //0 - success, 1 - fail
    bool isFatal = false;
    std::string error_text;
    const char* request = "ping\n";
    char bufer[64];
    ssize_t rec = 0;

    while (state != State::Done){
        switch(state){
            case State::Compose: { //send 
                ssize_t sent = send(file_descriptor, request, std::strlen(request), MSG_NOSIGNAL); //MSG_NOSIGNAL для возвращения понятной ошибки
                if (sent == -1){
                    error_text = std::strerror(errno);
                    isFatal = true;
                    state = State::HandleError;
                }
                else {
                    state = State::WaitingForReply;
                }
                break;
            }
            case State::WaitingForReply: { 
                rec = recv(file_descriptor, bufer, sizeof(bufer), 0);
                if (rec > 0){
                    state = State::ReadingReply;
                }
                else if (rec == 0){
                    error_text = "server closed connection";
                    isFatal = true;
                    state = State::HandleError;
                }
                else { // rec == -1
                    if (errno == EAGAIN || errno == EWOULDBLOCK){ //timeout related errors
                        error_text = "timeout";
                        isFatal = false;
                    }
                    else {
                        error_text = std::strerror(errno);
                        isFatal = true;
                    }
                    state = State::HandleError;
                }
                break;
            }
            case State::ReadingReply: { //recv
                std::string msg(bufer, rec);
                if (msg == "pong\n"){
                    std::cout << "received: pong" << "\n";
                    state = State::Done;
                }
                else{
                    error_text = "unexpected reply: " + msg;
                    isFatal = false;
                    state = State::HandleError;
                }
                break;
            }
            case State::HandleError: {
                std::cerr << "error: " << error_text << "\n";
                if (isFatal){
                    code_exit = 1;
                    state = State::Done;
                }
                else {
                    attempt++;
                    if (attempt >= max_attempts){
                        code_exit = 1;
                        state = State::Done;
                    }
                    else {
                        std::cout << "retry " << attempt << " of " << max_attempts << "\n";
                        state = State::Compose;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    close(file_descriptor);
    return code_exit;

}