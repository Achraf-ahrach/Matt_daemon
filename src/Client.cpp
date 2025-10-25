#include "Client.hpp"
#include "Auth.hpp"
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
    
    Auth auth;
    char buffer[4096];
    std::string currentDir;
    
    // Initialize current directory to daemon's working directory
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        currentDir = cwd;
    } else {
        currentDir = "/";
    }
    
    // Send login banner
    std::stringstream loginBanner;
    loginBanner << "\n";
    loginBanner << "========================================\n";
    loginBanner << "  Matt_daemon Remote Shell \n";
    loginBanner << "========================================\n";
    loginBanner << "Please login to continue.\n";
    loginBanner << "========================================\n\n";
    send(clientSock, loginBanner.str().c_str(), loginBanner.str().length(), MSG_NOSIGNAL);
    
    // Authentication loop
    bool authenticated = false;
    int attempts = 0;
    const int maxAttempts = 3;
    
    while (!authenticated && attempts < maxAttempts) {
        // Ask for username
        const char* usernamePrompt = "Username: ";
        send(clientSock, usernamePrompt, strlen(usernamePrompt), MSG_NOSIGNAL);
        
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            Tintin_reporter::log(INFO, "Matt_daemon: Client disconnected during login from " + clientInfo);
            close(clientSock);
            exit(0);
        }
        
        std::string username(buffer, bytes);
        username.erase(std::remove(username.begin(), username.end(), '\n'), username.end());
        username.erase(std::remove(username.begin(), username.end(), '\r'), username.end());
        
        // Ask for password
        const char* passwordPrompt = "Password: ";
        send(clientSock, passwordPrompt, strlen(passwordPrompt), MSG_NOSIGNAL);
        
        memset(buffer, 0, sizeof(buffer));
        bytes = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            Tintin_reporter::log(INFO, "Matt_daemon: Client disconnected during login from " + clientInfo);
            close(clientSock);
            exit(0);
        }
        
        std::string password(buffer, bytes);
        password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());
        password.erase(std::remove(password.begin(), password.end(), '\r'), password.end());
        
        // Authenticate
        if (auth.authenticate(username, password)) {
            authenticated = true;
            const char* successMsg = "\n✓ Login successful!\n\n";
            send(clientSock, successMsg, strlen(successMsg), MSG_NOSIGNAL);
            Tintin_reporter::log(INFO, "Matt_daemon: Client authenticated successfully from " + clientInfo);
        } else {
            attempts++;
            std::stringstream failMsg;
            failMsg << "\n✗ Authentication failed. ";
            if (attempts < maxAttempts) {
                failMsg << "Attempt " << attempts << "/" << maxAttempts << "\n\n";
            }
            send(clientSock, failMsg.str().c_str(), failMsg.str().length(), MSG_NOSIGNAL);
            Tintin_reporter::log(INFO, "Matt_daemon: Failed login attempt from " + clientInfo + " (attempt " + std::to_string(attempts) + ")");
        }
    }
    
    if (!authenticated) {
        const char* kickMsg = "\nMaximum login attempts exceeded. Connection closed.\n";
        send(clientSock, kickMsg, strlen(kickMsg), MSG_NOSIGNAL);
        Tintin_reporter::log(INFO, "Matt_daemon: Client kicked due to failed login attempts from " + clientInfo);
        close(clientSock);
        exit(0);
    }
    
    // Send welcome message and prompt
    std::stringstream welcome;
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