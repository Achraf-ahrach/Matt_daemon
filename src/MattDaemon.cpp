#include "MattDaemon.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <algorithm>

const std::string MattDaemon::LOCK_FILE = "/tmp/matt_daemon.lock";
const std::string MattDaemon::LOG_FILE = "/var/log/matt_daemon/matt_daemon.log";
MattDaemon* MattDaemon::_instance = nullptr;

MattDaemon::MattDaemon() : _serverSocket(-1), _running(false), _clientCount(0) {
    _instance = this;
}

MattDaemon::~MattDaemon() {
    shutdown();
}

MattDaemon* MattDaemon::getInstance() {
    return _instance;
}

bool MattDaemon::checkRootPrivileges() {
    if (getuid() != 0) {
        std::cerr << "Error: MattDaemon must be run with root privileges" << std::endl;
        return false;
    }
    return true;
}

bool MattDaemon::createLockFile() {
    std::ifstream lockCheck(LOCK_FILE);
    if (lockCheck.is_open()) {
        lockCheck.close();
        std::cerr << "Error: Another instance of MattDaemon is already running (lock file exists)" << std::endl;
        return false;
    }
    
    std::ofstream lockFile(LOCK_FILE);
    if (!lockFile.is_open()) {
        std::cerr << "Error: Cannot create lock file " << LOCK_FILE << std::endl;
        return false;
    }
    
    lockFile << getpid() << std::endl;
    lockFile.close();
    
    if (_logger) {
        _logger->logInfo("Lock file created: " + LOCK_FILE);
    }
    return true;
}

void MattDaemon::removeLockFile() {
    if (unlink(LOCK_FILE.c_str()) == 0) {
        if (_logger) {
            _logger->logInfo("Lock file removed: " + LOCK_FILE);
        }
    }
}

bool MattDaemon::daemonize() {
    pid_t pid = fork();
    
    if (pid < 0) {
        std::cerr << "Error: Fork failed" << std::endl;
        return false;
    }
    
    if (pid > 0) {
        exit(0);
    }
    
    if (setsid() < 0) {
        std::cerr << "Error: setsid failed" << std::endl;
        return false;
    }
    
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    pid = fork();
    if (pid < 0) {
        std::cerr << "Error: Second fork failed" << std::endl;
        return false;
    }
    
    if (pid > 0) {
        exit(0);
    }
    
    umask(0);
    chdir("/");
    
    for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--) {
        close(fd);
    }
    
    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);
    
    if (_logger) {
        _logger->logInfo("Daemon process created successfully");
    }
    
    return true;
}

bool MattDaemon::setupSocket() {
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0) {
        if (_logger) {
            _logger->logError("Failed to create socket: " + std::string(strerror(errno)));
        }
        return false;
    }
    
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        if (_logger) {
            _logger->logError("Failed to set socket options: " + std::string(strerror(errno)));
        }
        return false;
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(_serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        if (_logger) {
            _logger->logError("Failed to bind socket to port " + std::to_string(PORT) + ": " + std::string(strerror(errno)));
        }
        return false;
    }
    
    if (listen(_serverSocket, MAX_CLIENTS) < 0) {
        if (_logger) {
            _logger->logError("Failed to listen on socket: " + std::string(strerror(errno)));
        }
        return false;
    }
    
    if (_logger) {
        _logger->logInfo("Socket listening on port " + std::to_string(PORT));
    }
    
    return true;
}

void MattDaemon::handleClient(int clientSocket) {
    _clientCount++;
    if (_logger) {
        _logger->logInfo("Client connected. Active clients: " + std::to_string(_clientCount.load()));
    }
    
    char buffer[1024];
    while (_running) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) {
            break;
        }
        
        std::string message(buffer, bytesRead);
        message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());
        message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());
        
        if (_logger) {
            _logger->logInfo("Received message: " + message);
        }
        
        if (message == "quit") {
            if (_logger) {
                _logger->logInfo("Quit command received, shutting down daemon");
            }
            _running = false;
            break;
        }
    }
    
    close(clientSocket);
    _clientCount--;
    if (_logger) {
        _logger->logInfo("Client disconnected. Active clients: " + std::to_string(_clientCount.load()));
    }
}

void MattDaemon::cleanupClientThreads() {
    for (auto& thread : _clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    _clientThreads.clear();
}

void MattDaemon::signalHandler(int signal) {
    if (_instance && _instance->_logger) {
        std::string signalName;
        switch (signal) {
            case SIGTERM: signalName = "SIGTERM"; break;
            case SIGINT: signalName = "SIGINT"; break;
            case SIGQUIT: signalName = "SIGQUIT"; break;
            case SIGUSR1: signalName = "SIGUSR1"; break;
            case SIGUSR2: signalName = "SIGUSR2"; break;
            default: signalName = "Signal " + std::to_string(signal); break;
        }
        _instance->_logger->logInfo("Received signal " + signalName + ", shutting down gracefully");
    }
    
    if (_instance) {
        _instance->_running = false;
    }
}

void MattDaemon::setupSignalHandlers() {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    
    if (_logger) {
        _logger->logInfo("Signal handlers configured");
    }
}

bool MattDaemon::initialize() {
    if (!checkRootPrivileges()) {
        return false;
    }
    
    _logger.reset(new Tintin_reporter(LOG_FILE));
    if (!_logger->isOpen()) {
        std::cerr << "Error: Cannot open log file" << std::endl;
        return false;
    }
    
    _logger->logInfo("MattDaemon initialization started");
    
    if (!createLockFile()) {
        return false;
    }
    
    if (!daemonize()) {
        removeLockFile();
        return false;
    }
    
    setupSignalHandlers();
    
    if (!setupSocket()) {
        removeLockFile();
        return false;
    }
    
    _logger->logInfo("MattDaemon initialized successfully");
    return true;
}

void MattDaemon::run() {
    _running = true;
    _logger->logInfo("MattDaemon started and listening for connections");
    
    while (_running) {
        struct sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(_serverSocket, &readfds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(_serverSocket + 1, &readfds, nullptr, nullptr, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            _logger->logError("Select error: " + std::string(strerror(errno)));
            break;
        }
        
        if (activity > 0 && FD_ISSET(_serverSocket, &readfds)) {
            int clientSocket = accept(_serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
            
            if (clientSocket < 0) {
                if (errno != EINTR) {
                    _logger->logError("Accept failed: " + std::string(strerror(errno)));
                }
                continue;
            }
            
            if (_clientCount >= MAX_CLIENTS) {
                _logger->logWarning("Maximum clients reached, rejecting connection");
                close(clientSocket);
                continue;
            }
            
            cleanupClientThreads();
            _clientThreads.emplace_back(&MattDaemon::handleClient, this, clientSocket);
        }
    }
    
    cleanupClientThreads();
}

void MattDaemon::shutdown() {
    _running = false;
    
    if (_logger) {
        _logger->logInfo("MattDaemon shutting down");
    }
    
    if (_serverSocket >= 0) {
        close(_serverSocket);
        _serverSocket = -1;
    }
    
    cleanupClientThreads();
    removeLockFile();
    
    if (_logger) {
        _logger->logInfo("MattDaemon shutdown complete");
    }
}
