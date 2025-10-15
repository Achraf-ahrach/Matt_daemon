#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <message>" << std::endl;
        return 1;
    }
    
    std::string message = argv[1];
    
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Error: Cannot create socket" << std::endl;
        return 1;
    }
    
    // Setup server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4242);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Connect to server
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Cannot connect to MattDaemon on port 4242" << std::endl;
        close(sock);
        return 1;
    }
    
    std::cout << "Connected to MattDaemon" << std::endl;
    
    // Send message
    if (send(sock, message.c_str(), message.length(), 0) < 0) {
        std::cerr << "Error: Failed to send message" << std::endl;
    } else {
        std::cout << "Message sent: " << message << std::endl;
    }
    
    // Close connection
    close(sock);
    std::cout << "Connection closed" << std::endl;
    
    return 0;
}
