#ifndef SIMPLESERVER_HPP
#define SIMPLESERVER_HPP

#include "SimpleLogger.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <string>

class Server {
private:
    static const int PORT = 9999;
    static const int MAX_CLIENTS = 3;
    
    int serverSocket;
    std::atomic<bool> running;
    std::atomic<int> clientCount;
    std::vector<std::thread> clientThreads;
    SimpleLogger logger;
    
    void handleClient(int clientSocket, std::string clientIP);
    void cleanupThreads();
    
public:
    SimpleServer();
    ~SimpleServer();
    
    bool start();
    void run();
    void stop();
};

#endif
