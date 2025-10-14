#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <cstring>

class SimpleLogger {
private:
    std::string logFile;
    
    std::string getCurrentTimestamp() const {
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);
        
        std::ostringstream oss;
        oss << "[ " << std::setfill('0') << std::setw(2) << localTime->tm_mday
            << " / " << std::setw(2) << (localTime->tm_mon + 1)
            << " / " << (localTime->tm_year + 1900)
            << " - " << std::setw(2) << localTime->tm_hour
            << " : " << std::setw(2) << localTime->tm_min
            << " : " << std::setw(2) << localTime->tm_sec << " ]";
        
        return oss.str();
    }
    
public:
    SimpleLogger(const std::string& file) : logFile(file) {}
    
    void log(const std::string& message) {
        std::ofstream file(logFile, std::ios::app);
        if (file.is_open()) {
            file << getCurrentTimestamp() << " " << message << std::endl;
            file.close();
        }
    }
};

class SimpleServer {
private:
    static const int PORT = 9999;
    static const int MAX_CLIENTS = 3;
    
    int serverSocket;
    std::atomic<bool> running;
    std::atomic<int> clientCount;
    std::vector<std::thread> clientThreads;
    SimpleLogger logger;
    
    void handleClient(int clientSocket, std::string clientIP) {
        clientCount++;
        logger.log("[INFO] Client connected from " + clientIP + ". Active clients: " + std::to_string(clientCount.load()));
        
        char buffer[1024];
        while (running) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            
            if (bytesRead <= 0) {
                break;
            }
            
            std::string message(buffer, bytesRead);
            // Remove newlines
            message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());
            message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());
            
            if (!message.empty()) {
                logger.log("[MESSAGE] From " + clientIP + ": " + message);
                
                if (message == "quit") {
                    logger.log("[INFO] Quit command received from " + clientIP);
                    break;
                }
            }
        }
        
        close(clientSocket);
        clientCount--;
        logger.log("[INFO] Client " + clientIP + " disconnected. Active clients: " + std::to_string(clientCount.load()));
    }
    
    void cleanupThreads() {
        for (auto& thread : clientThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        clientThreads.clear();
    }
    
public:
    SimpleServer() : serverSocket(-1), running(false), clientCount(0), logger("server.log") {}
    
    ~SimpleServer() {
        stop();
    }
    
    bool start() {
        // Create socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Error: Cannot create socket" << std::endl;
            return false;
        }
        
        // Set socket options
        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Error: Cannot set socket options" << std::endl;
            return false;
        }
        
        // Bind socket
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);
        
        if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Error: Cannot bind to port " << PORT << std::endl;
            return false;
        }
        
        // Listen
        if (listen(serverSocket, MAX_CLIENTS) < 0) {
            std::cerr << "Error: Cannot listen on socket" << std::endl;
            return false;
        }
        
        running = true;
        logger.log("[INFO] Server started on port " + std::to_string(PORT));
        std::cout << "Server started on port " << PORT << std::endl;
        
        return true;
    }
    
    void run() {
        while (running) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(serverSocket, &readfds);
            
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            int activity = select(serverSocket + 1, &readfds, nullptr, nullptr, &timeout);
            
            if (activity > 0 && FD_ISSET(serverSocket, &readfds)) {
                int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
                
                if (clientSocket < 0) {
                    continue;
                }
                
                std::string clientIP = inet_ntoa(clientAddr.sin_addr);
                
                if (clientCount >= MAX_CLIENTS) {
                    logger.log("[WARNING] Max clients reached, rejecting connection from " + clientIP);
                    close(clientSocket);
                    continue;
                }
                
                // Clean up finished threads
                cleanupThreads();
                
                // Start new client thread
                clientThreads.emplace_back(&SimpleServer::handleClient, this, clientSocket, clientIP);
            }
        }
        
        cleanupThreads();
    }
    
    void stop() {
        running = false;
        if (serverSocket >= 0) {
            close(serverSocket);
            serverSocket = -1;
        }
        cleanupThreads();
        logger.log("[INFO] Server stopped");
    }
};

// Global server instance for signal handling
SimpleServer* globalServer = nullptr;

void signalHandler(int signal) {
    if (globalServer) {
        std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
        globalServer->stop();
    }
}

int main() {
    SimpleServer server;
    globalServer = &server;
    
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    // Detach from terminal (run in background)
    if (fork() == 0) {
        // Child process - run the server
        setsid();  // Create new session
        server.run();
    } else {
        // Parent process - exit
        std::cout << "Server detached and running in background" << std::endl;
        std::cout << "Check server.log for activity" << std::endl;
        std::cout << "Connect with: nc localhost 4242" << std::endl;
    }
    
    return 0;
}
