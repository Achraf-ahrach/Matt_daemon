#include "Tintin_reporter.hpp"
#include "Utils.hpp"
#include "Daemon.hpp"
#include "Server.hpp"
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <sys/file.h>

int main() {
    if (getuid() != 0) {
        std::cerr << "Error: Matt_daemon must be run with root privileges" << std::endl;
        return 1;
    }
    
    Tintin_reporter::log(INFO, "Matt_daemon: Started.");
    
    int lockFd = -1;
    if (!Utils::createLockFile(lockFd)) {
        std::cerr << "Error: Matt_daemon instance already running" << std::endl;
        Tintin_reporter::log(INFO, "Matt_daemon: Quitting.");
        return 1;
    }
    
    Server server;
    if (!server.initialize()) {
        Utils::cleanup(server.getSocket(), lockFd);
        return 1;
    }
    
    Tintin_reporter::log(INFO, "Matt_daemon: Entering Daemon mode.");
    Daemon::daemonize();
    
    if (lockFd >= 0) {
        lseek(lockFd, 0, SEEK_SET);
        ftruncate(lockFd, 0);
        std::string pidStr = std::to_string(getpid()) + "\n";
        write(lockFd, pidStr.c_str(), pidStr.length());
    }

    server.setLockFd(lockFd);
    
    std::stringstream ss;
    ss << "Matt_daemon: started. PID: " << getpid() << ".";
    Tintin_reporter::log(INFO, ss.str());
    
    Daemon::setupSignalHandlers();
    server.run();

    return 0;
}