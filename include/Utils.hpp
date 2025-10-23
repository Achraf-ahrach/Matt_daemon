#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

#define LOCK_FILE "/var/lock/matt_daemon.lock"
#define PORT 4242
#define MAX_CLIENTS 3

#define EMAIL_FROM "achrafahrach44@gmail.com"
#define EMAIL_TO   "achrafahrach44@gmail.com"
#define EMAIL_PASS "myhh aaxy qual heup"

namespace Utils {
    void sendExitEmail(const std::string& reason, const std::string& clientInfo = "");
    bool createLockFile(int& lockFd);
    void cleanup(int serverSocket, int lockFd);
}

#endif