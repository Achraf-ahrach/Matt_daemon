#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

class Tintin_reporter {
private:
    std::string logPath;
    
    std::string getCurrentTimestamp() const {
        std::time_t now = std::time(nullptr);
        std::tm* t = std::localtime(&now);
        std::ostringstream oss;
        oss << "[" << std::setfill('0') << std::setw(2) << t->tm_mday
            << "/" << std::setw(2) << (t->tm_mon + 1)
            << "/" << (t->tm_year + 1900)
            << "-" << std::setw(2) << t->tm_hour
            << ":" << std::setw(2) << t->tm_min
            << ":" << std::setw(2) << t->tm_sec << "]";
        return oss.str();
    }
    
    void ensureLogDirectory() const {
        mkdir("/var/log/matt_daemon", 0755);
    }

public:
    // Coplien form (Orthodox Canonical Class Form)
    
    // Default constructor
    Tintin_reporter(const std::string& logFile = "/var/log/matt_daemon/matt_daemon.log") 
        : logPath(logFile) {
        ensureLogDirectory();
    }
    
    // Copy constructor
    Tintin_reporter(const Tintin_reporter& other) : logPath(other.logPath) {}
    
    // Assignment operator
    Tintin_reporter& operator=(const Tintin_reporter& other) {
        if (this != &other) {
            logPath = other.logPath;
        }
        return *this;
    }
    
    // Destructor
    ~Tintin_reporter() {}
    
    // Logging methods
    void writeLog(const std::string& message) const {
        std::ofstream file(logPath.c_str(), std::ios::app);
        if (file.is_open()) {
            file << getCurrentTimestamp() << " " << message << std::endl;
            file.close();
        }
    }
    
    // Static instance for easy access
    static Tintin_reporter& getInstance() {
        static Tintin_reporter instance;
        return instance;
    }
    
    // Static convenience method
    static void log(const std::string& message) {
        getInstance().writeLog(message);
    }
};

#endif
