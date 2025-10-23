#include "Client.hpp"
#include "Utils.hpp"
#include "Tintin_reporter.hpp"
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <cstring>
#include <algorithm>

void Client::signalHandler(int sig) {
    std::string sigName;
    switch(sig) {
        case SIGTERM: sigName = "SIGTERM"; break;
        case SIGINT: sigName = "SIGINT"; break;
        case SIGQUIT: sigName = "SIGQUIT"; break;
        case SIGHUP: sigName = "SIGHUP (Client disconnected)"; break;
        case SIGPIPE: sigName = "SIGPIPE (Broken pipe)"; break;
        default: sigName = "UNKNOWN"; break;
    }
    
    Tintin_reporter::log(INFO, "Matt_daemon: Client handler received signal " + sigName);
    Utils::sendExitEmail("Client handler terminated by signal: " + sigName);
    exit(0);
}

void Client::setupSignalHandlers() {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGPIPE, signalHandler);
}

void Client::handleClient(int clientSock, const std::string& clientInfo) {
    setupSignalHandlers();
    
    char buffer[4096];
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes <= 0) {
            if (bytes == 0) {
                Tintin_reporter::log(INFO, "Matt_daemon: Client disconnected normally from " + clientInfo);
                Utils::sendExitEmail("Client disconnected normally", clientInfo);
            } else {
                Tintin_reporter::log(ERROR, "Matt_daemon: Client connection error from " + clientInfo);
                Utils::sendExitEmail("Client connection error (recv failed)", clientInfo);
            }
            break;
        }
        
        std::string msg(buffer, bytes);
        msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
        msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
        
        if (!msg.empty()) {
            if (msg == "quit") {
                Tintin_reporter::log(INFO, "Matt_daemon: Request quit from " + clientInfo);
                Utils::sendExitEmail("Client sent 'quit' command", clientInfo);
                close(clientSock);
                kill(getppid(), SIGTERM);
                exit(0);
            } else {
                Tintin_reporter::log(LOG, "Matt_daemon: User input: " + msg);
            }
        }
    }
    
    close(clientSock);
    exit(0);
}