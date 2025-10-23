#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

#define LOCK_FILE "/var/lock/matt_daemon.lock"
#define PORT 4242
#define MAX_CLIENTS 3

namespace Utils {
    void sendExitEmail(const std::string& reason, const std::string& clientInfo = "");
    bool createLockFile(int& lockFd);
    void cleanup(int serverSocket, int lockFd);
}

#endif