#include "Utils.hpp"
#include "Tintin_reporter.hpp"
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstring>

void Utils::sendExitEmail(const std::string& reason, const std::string& clientInfo) {
    const char* emailFrom = EMAIL_FROM;
    const char* emailPass = EMAIL_PASS;
    const char* emailTo   = EMAIL_TO;

    std::time_t now = std::time(nullptr);
    now += 3600; // add +1 hour (for local time adjustment)

    char timestamp[100];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    
    std::ofstream emailFile("/tmp/matt_daemon_email.txt");
    if (emailFile.is_open()) {
        emailFile << "From: Matt Daemon <" << emailFrom << ">\n";
        emailFile << "To: <" << emailTo << ">\n";
        emailFile << "Subject: Matt_daemon Client Exit Notification\n\n";
        emailFile << "A client has disconnected from Matt_daemon.\n";
        emailFile << "Timestamp: " << timestamp << "\n";
        emailFile << "Daemon PID: " << getppid() << "\n";
        emailFile << "Client Handler PID: " << getpid() << "\n";
        if (!clientInfo.empty()) {
            emailFile << "Client Info: " << clientInfo << "\n";
        }
        emailFile << "Reason: " << reason << "\n";
        emailFile.close();
        
        pid_t emailPid = fork();
        if (emailPid == 0) {
            std::string userAuth = std::string(emailFrom) + ":" + emailPass;
            execl("/usr/bin/curl", "curl",
                "--url", "smtps://smtp.gmail.com:465",
                "--ssl-reqd",
                "--mail-from", emailFrom,
                "--mail-rcpt", emailTo,
                "--user", userAuth.c_str(),
                "--upload-file", "/tmp/matt_daemon_email.txt",
                (char*)NULL
            );
            exit(1);
        } else if (emailPid > 0) {
            waitpid(emailPid, NULL, WNOHANG);
        }
    } else {
        Tintin_reporter::log(ERROR, "Failed to open /tmp/matt_daemon_email.txt for writing.");
    }
}

bool Utils::createLockFile(int& lockFd) {
    lockFd = open(LOCK_FILE, O_CREAT | O_RDWR, 0644);
    if (lockFd < 0) {
        Tintin_reporter::log(ERROR, "Matt_daemon: Error opening lock file.");
        return false;
    }
    
    if (flock(lockFd, LOCK_EX | LOCK_NB) < 0) {
        Tintin_reporter::log(ERROR, "Matt_daemon: Instance already running.");
        close(lockFd);
        lockFd = -1;
        return false;
    }
    
    std::string pidStr = std::to_string(getpid()) + "\n";
    write(lockFd, pidStr.c_str(), pidStr.length());
    
    return true;
}

void Utils::cleanup(int serverSocket, int lockFd) {
    Tintin_reporter::log(INFO, "Matt_daemon: Quitting.");
    if (serverSocket >= 0) {
        close(serverSocket);
    }
    if (lockFd >= 0) {
        close(lockFd);
        unlink(LOCK_FILE);
    }
}