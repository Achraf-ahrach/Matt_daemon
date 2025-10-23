#include "Client.hpp"
#include "Utils.hpp"
#include "Tintin_reporter.hpp"
#include "ShellCommands.hpp"
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <cstring>
#include <algorithm>
#include <sstream>

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
    std::string currentDir;
    
    // Initialize current directory to daemon's working directory
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        currentDir = cwd;
    } else {
        currentDir = "/";
    }
    
    // Send welcome message and prompt
    std::stringstream welcome;
    welcome << "\n";
    welcome << "========================================\n";
    welcome << "  Matt_daemon Remote Shell \n";
    welcome << "========================================\n";
    welcome << "Type 'help' for available commands.\n";
    welcome << "========================================\n\n";
    send(clientSock, welcome.str().c_str(), welcome.str().length(), MSG_NOSIGNAL);
    
    // Send initial prompt with colored "matt_daemon"
    std::string prompt = "\033[1;36mmatt_daemon\033[0m:" + currentDir + "$ ";
    send(clientSock, prompt.c_str(), prompt.length(), MSG_NOSIGNAL);
    
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
            // Log the user input
            Tintin_reporter::log(LOG, "Matt_daemon: User input: " + msg);
            
            // Execute the shell command
            std::string response = ShellCommands::executeCommand(msg, currentDir);
            
            // Send response to client
            if (!response.empty()) {
                send(clientSock, response.c_str(), response.length(), MSG_NOSIGNAL);
            }
            
            // Send new prompt with updated directory (colored "matt_daemon")
            prompt = "\033[1;36mmatt_daemon\033[0m:" + currentDir + "$ ";
            send(clientSock, prompt.c_str(), prompt.length(), MSG_NOSIGNAL);
        } else {
            // Empty command, just send prompt again (colored "matt_daemon")
            prompt = "\033[1;36mmatt_daemon\033[0m:" + currentDir + "$ ";
            send(clientSock, prompt.c_str(), prompt.length(), MSG_NOSIGNAL);
        }
    }
    
    close(clientSock);
    exit(0);
}