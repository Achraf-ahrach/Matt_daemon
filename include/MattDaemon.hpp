#ifndef MATTDAEMON_HPP
#define MATTDAEMON_HPP

#include "Tintin_reporter.hpp"
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>

class MattDaemon {
private:
    static const int PORT = 4242;
    static const int MAX_CLIENTS = 3;
    static const std::string LOCK_FILE;
    static const std::string LOG_FILE;
    
    std::unique_ptr<Tintin_reporter> _logger;
    int _serverSocket;
    std::atomic<bool> _running;
    std::vector<std::thread> _clientThreads;
    std::atomic<int> _clientCount;
    
    static MattDaemon* _instance;
    
    bool checkRootPrivileges();
    bool createLockFile();
    void removeLockFile();
    bool daemonize();
    bool setupSocket();
    void handleClient(int clientSocket);
    void cleanupClientThreads();
    
    static void signalHandler(int signal);
    void setupSignalHandlers();
    
public:
    MattDaemon();
    ~MattDaemon();
    
    bool initialize();
    void run();
    void shutdown();
    
    static MattDaemon* getInstance();
};

#endif