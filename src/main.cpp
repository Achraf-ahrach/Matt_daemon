#include "Tintin_reporter.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

bool running = true;
int serverSocket = -1;
int clientCount = 0;

void signalHandler(int sig) {
    std::string sigName = (sig == SIGTERM) ? "SIGTERM" : 
                         (sig == SIGINT) ? "SIGINT" : 
                         (sig == SIGQUIT) ? "SIGQUIT" : 
                         (sig == SIGUSR1) ? "SIGUSR1" : 
                         (sig == SIGUSR2) ? "SIGUSR2" : "UNKNOWN";
    
    Tintin_reporter::log("[SIGNAL] Received " + sigName + " - shutting down gracefully");
    running = false;
    if (serverSocket >= 0) close(serverSocket);
    unlink("/var/lock/matt_daemon.lock");
    exit(0);
}

void handleClient(int clientSock, std::string clientIP) {
    clientCount++;
    Tintin_reporter::log("[INFO] Client connected from " + clientIP + ". Active clients: " + std::to_string(clientCount));
    
    char buffer[1024];
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        
        std::string msg(buffer, bytes);
        msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
        msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
        
        if (!msg.empty()) {
            if (msg == "quit") {
                Tintin_reporter::log("[INFO] Quit command received from " + clientIP);
                running = false;
                break;
            } else {
                Tintin_reporter::log("Message from " + clientIP + ": " + msg);
            }
        }
    }
    
    close(clientSock);
    clientCount--;
    Tintin_reporter::log("[INFO] Client " + clientIP + " disconnected. Active clients: " + std::to_string(clientCount));
}

int main() {
    // Check root privileges
    if (getuid() != 0) {
        std::cerr << "Error: MattDaemon must be run with root privileges (use sudo)" << std::endl;
        return 1;
    }
    
    // Check lock file
    std::ifstream lockCheck("/var/lock/matt_daemon.lock");
    if (lockCheck.good()) {
        lockCheck.close();
        std::cerr << "Error: Another instance of MattDaemon is already running" << std::endl;
        std::cerr << "Error: Cannot create/open /var/lock/matt_daemon.lock" << std::endl;
        return 1;
    }
    
    // Create lock file
    std::ofstream lockFile("/var/lock/matt_daemon.lock");
    if (!lockFile.is_open()) {
        std::cerr << "Error: Cannot create lock file /var/lock/matt_daemon.lock" << std::endl;
        return 1;
    }
    lockFile << getpid() << std::endl;
    lockFile.close();
    
    Tintin_reporter::log("[INFO] Starting MattDaemon...");
    
    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        Tintin_reporter::log("[ERROR] Cannot create socket");
        unlink("/var/lock/matt_daemon.lock");
        return 1;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(4242);
    
    if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        Tintin_reporter::log("[ERROR] Cannot bind to port 4242");
        close(serverSocket);
        unlink("/var/lock/matt_daemon.lock");
        return 1;
    }
    
    if (listen(serverSocket, 3) < 0) {
        Tintin_reporter::log("[ERROR] Cannot listen on socket");
        close(serverSocket);
        unlink("/var/lock/matt_daemon.lock");
        return 1;
    }
    
    // Daemonize
    if (fork() != 0) exit(0);  // Parent exits
    setsid();                  // New session
    if (fork() != 0) exit(0);  // Parent exits again
    chdir("/");                // Change to root directory
    umask(0);                  // Clear file mode mask
    
    // Close standard descriptors and redirect to /dev/null
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
    
    // Setup signal handlers
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    
    Tintin_reporter::log("[INFO] MattDaemon started successfully on port 4242");
    
    // Main server loop
    while (running) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        
        struct timeval timeout = {1, 0};
        int activity = select(serverSocket + 1, &readfds, nullptr, nullptr, &timeout);
        
        if (activity > 0 && FD_ISSET(serverSocket, &readfds)) {
            int clientSock = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSock < 0) continue;
            
            std::string clientIP = inet_ntoa(clientAddr.sin_addr);
            
            if (clientCount >= 3) {
                Tintin_reporter::log("[WARNING] Max clients (3) reached, rejecting connection from " + clientIP);
                close(clientSock);
                continue;
            }
            
            // Handle client in a simple way (not using threads for simplicity)
            if (fork() == 0) {
                close(serverSocket);
                handleClient(clientSock, clientIP);
                exit(0);
            }
            close(clientSock);
        }
    }
    
    close(serverSocket);
    unlink("/var/lock/matt_daemon.lock");
    Tintin_reporter::log("[INFO] MattDaemon stopped");
    
    return 0;
}
