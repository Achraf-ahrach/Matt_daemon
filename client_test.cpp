#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <message>" << std::endl;
        std::cout << "Example: " << argv[0] << " \"Hello MattDaemon\"" << std::endl;
        std::cout << "Special: " << argv[0] << " quit" << std::endl;
        return 1;
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4242);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error connecting to daemon on port 4242" << std::endl;
        std::cerr << "Make sure MattDaemon is running with: sudo ./MattDaemon" << std::endl;
        close(sock);
        return 1;
    }
    
    std::string message = argv[1];
    if (send(sock, message.c_str(), message.length(), 0) < 0) {
        std::cerr << "Error sending message" << std::endl;
        close(sock);
        return 1;
    }
    
    std::cout << "Message sent: \"" << message << "\"" << std::endl;
    
    if (message == "quit") {
        std::cout << "Quit command sent - daemon should shutdown" << std::endl;
    } else {
        std::cout << "Check log file: sudo tail /var/log/matt_daemon/matt_daemon.log" << std::endl;
    }
    
    close(sock);
    return 0;
}
