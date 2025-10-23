#include "Server.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include "Daemon.hpp"
#include "Tintin_reporter.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include <sys/select.h>
#include <errno.h>
#include <sstream>

Server::Server() : serverSocket(-1), lockFd(-1), activeClients(0) {}

Server::~Server() {
    Utils::cleanup(serverSocket, lockFd);
}

bool Server::initialize() {
    Tintin_reporter::log(INFO, "Matt_daemon: Creating server.");
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        Tintin_reporter::log(ERROR, "Matt_daemon: Cannot create socket.");
        return false;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        Tintin_reporter::log(ERROR, "Matt_daemon: Cannot bind to port 4242.");
        return false;
    }
    
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        Tintin_reporter::log(ERROR, "Matt_daemon: Cannot listen on socket.");
        return false;
    }
    
    Tintin_reporter::log(INFO, "Matt_daemon: Server created.");
    return true;
}

void Server::run() {
    while (running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(serverSocket + 1, &readfds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            continue;
        }
        
        if (activity > 0 && FD_ISSET(serverSocket, &readfds)) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            
            int clientSock = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSock < 0) {
                continue;
            }
            
            if (activeClients >= MAX_CLIENTS) {
                Tintin_reporter::log(ERROR, "Matt_daemon: Maximum clients reached, connection rejected.");
                const char* msg = "Server full: Maximum 3 clients allowed. Connection closed.\n";
                send(clientSock, msg, strlen(msg), MSG_NOSIGNAL);
                close(clientSock);
                continue;
            }
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            std::string clientInfo = std::string(clientIP) + ":" + std::to_string(ntohs(clientAddr.sin_port));
            
            pid_t clientPid = fork();
            if (clientPid == 0) {
                close(serverSocket);
                Client::handleClient(clientSock, clientInfo);
            } else if (clientPid > 0) {
                close(clientSock);
                activeClients++;
                Tintin_reporter::log(INFO, "Matt_daemon: Client connected from " + clientInfo);
            }
        }
        
        int status;
        while (waitpid(-1, &status, WNOHANG) > 0) {
            activeClients--;
            if (activeClients < 0) activeClients = 0;
        }
    }
}