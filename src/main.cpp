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
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <fcntl.h>
#include <errno.h>

#define LOCK_FILE "/var/lock/matt_daemon.lock"
#define PORT 4242
#define MAX_CLIENTS 3

bool running = true;
int serverSocket = -1;
int lockFd = -1;

void cleanup() {
    Tintin_reporter::log("[ INFO ] - Matt_daemon: Quitting.");
    if (serverSocket >= 0) {
        close(serverSocket);
    }
    if (lockFd >= 0) {
        close(lockFd);
        unlink(LOCK_FILE);
    }
}

void signalHandler(int sig) {
    std::string sigName;
    switch(sig) {
        case SIGTERM: sigName = "SIGTERM"; break;
        case SIGINT: sigName = "SIGINT"; break;
        case SIGQUIT: sigName = "SIGQUIT"; break;
        case SIGUSR1: sigName = "SIGUSR1"; break;
        case SIGUSR2: sigName = "SIGUSR2"; break;
        default: sigName = "UNKNOWN"; break;
    }
    
    Tintin_reporter::log("[ INFO ] - Matt_daemon: Signal handler.");
    running = false;
}

bool createLockFile() {
    lockFd = open(LOCK_FILE, O_CREAT | O_RDWR, 0644);
    if (lockFd < 0) {
        std::cerr << "Can't open :" << LOCK_FILE << std::endl;
        Tintin_reporter::log("[ ERROR ] - Matt_daemon: Error file locked.");
        return false;
    }
    
    // Try to acquire exclusive lock
    if (flock(lockFd, LOCK_EX | LOCK_NB) < 0) {
        std::cerr << "Can't open :" << LOCK_FILE << std::endl;
        Tintin_reporter::log("[ ERROR ] - Matt_daemon: Error file locked.");
        close(lockFd);
        lockFd = -1;
        return false;
    }
    
    // Write PID to lock file
    std::string pidStr = std::to_string(getpid()) + "\n";
    write(lockFd, pidStr.c_str(), pidStr.length());
    
    return true;
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);  // Parent exits
    }
    
    // Child continues
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Second fork
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);  // Parent exits
    }
    
    // Change working directory
    chdir("/");
    
    // Clear file mode mask
    umask(0);
    
    // Close all standard file descriptors and redirect to /dev/null
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    open("/dev/null", O_RDONLY);  // stdin
    open("/dev/null", O_WRONLY);  // stdout
    open("/dev/null", O_WRONLY);  // stderr
}

int main() {
    // Check root privileges
    if (getuid() != 0) {
        std::cerr << "Error: Matt_daemon must be run with root privileges" << std::endl;
        return 1;
    }
    
    Tintin_reporter::log("[ INFO ] - Matt_daemon: Started.");
    
    // Create lock file (before daemonizing for error reporting)
    if (!createLockFile()) {
        Tintin_reporter::log("[ INFO ] - Matt_daemon: Quitting.");
        return 1;
    }
    
    Tintin_reporter::log("[ INFO ] - Matt_daemon: Creating server.");
    
    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        Tintin_reporter::log("[ ERROR ] - Matt_daemon: Cannot create socket.");
        cleanup();
        return 1;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        Tintin_reporter::log("[ ERROR ] - Matt_daemon: Cannot bind to port 4242.");
        cleanup();
        return 1;
    }
    
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        Tintin_reporter::log("[ ERROR ] - Matt_daemon: Cannot listen on socket.");
        cleanup();
        return 1;
    }
    
    Tintin_reporter::log("[ INFO ] - Matt_daemon: Server created.");
    Tintin_reporter::log("[ INFO ] - Matt_daemon: Entering Daemon mode.");
    
    // Daemonize the process
    daemonize();
    
    // Update lock file with new PID after daemonizing
    if (lockFd >= 0) {
        lseek(lockFd, 0, SEEK_SET);
        ftruncate(lockFd, 0);
        std::string pidStr = std::to_string(getpid()) + "\n";
        write(lockFd, pidStr.c_str(), pidStr.length());
    }
    
    std::stringstream ss;
    ss << "[ INFO ] - Matt_daemon: started. PID: " << getpid() << ".";
    Tintin_reporter::log(ss.str());
    
    // Setup signal handlers
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    
    // Track active connections
    int activeClients = 0;
    
    // Main server loop
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
                Tintin_reporter::log("[ WARNING ] - Matt_daemon: Maximum clients reached, connection rejected.");
                const char* msg = "Server full: Maximum 3 clients allowed. Connection closed.\n";
                send(clientSock, msg, strlen(msg), MSG_NOSIGNAL);
                close(clientSock);
                continue;
            }
            
            // Fork to handle client
            pid_t clientPid = fork();
            if (clientPid == 0) {
                // Child process handles client
                close(serverSocket);
                
                char buffer[4096];
                while (true) {
                    memset(buffer, 0, sizeof(buffer));
                    ssize_t bytes = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
                    
                    if (bytes <= 0) {
                        break;
                    }
                    
                    std::string msg(buffer, bytes);
                    // Remove newlines and carriage returns
                    msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
                    msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
                    
                    if (!msg.empty()) {
                        if (msg == "quit") {
                            Tintin_reporter::log("[ INFO ] - Matt_daemon: Request quit.");
                            close(clientSock);
                            // Signal parent to quit
                            kill(getppid(), SIGTERM);
                            exit(0);
                        } else {
                            Tintin_reporter::log("[ LOG ] - Matt_daemon: User input: " + msg);
                        }
                    }
                }
                
                close(clientSock);
                exit(0);
            } else if (clientPid > 0) {
                // Parent process
                close(clientSock);
                activeClients++;
            }
        }
        
        // Reap zombie child processes
        int status;
        while (waitpid(-1, &status, WNOHANG) > 0) {
            activeClients--;
            if (activeClients < 0) activeClients = 0;
        }
    }
    
    cleanup();
    return 0;
}
