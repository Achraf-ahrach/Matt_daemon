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

#define LOCK_FILE "/var/lock/matt_daemon.lock"
#define PORT 4242
#define MAX_CLIENTS 3

enum level {
    ERROR,
    LOG,
    INFO
};
#define BASE_PATH  "/var/log/matt_daemon"
#define ERROR_PATH  "/var/log/matt_daemon/Error.log"
#define LOG_PATH  "/var/log/matt_daemon/Log.log"
#define INFO_PATH  "/var/log/matt_daemon/Info.log"

class Tintin_reporter {
private:
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
        mkdir(BASE_PATH, 0755);
    }
    
    const char* getLogPath(level logLevel) const {
        switch (logLevel) {
            case ERROR:
                return ERROR_PATH;
            case LOG:
                return LOG_PATH;
            case INFO:
                return INFO_PATH;
            default:
                return LOG_PATH;
        }
    }
    
    std::string getLevelString(level logLevel) const {
        switch (logLevel) {
            case ERROR:
                return "[ ERROR ]";
            case LOG:
                return "[ LOG ]";
            case INFO:
                return "[ INFO ]";
            default:
                return "[ LOG ]";
        }
    }

public:
    // Coplien form (Orthodox Canonical Class Form)
    
    // Default constructor
    Tintin_reporter() {
        ensureLogDirectory();
    }
    
    // Copy constructor
    Tintin_reporter(const Tintin_reporter& other) {
        (void)other;
    }
    
    // Assignment operator
    Tintin_reporter& operator=(const Tintin_reporter& other) {
        (void)other;
        return *this;
    }
    
    // Destructor
    ~Tintin_reporter() {}
    
    // Logging methods with level support
    void writeLog(level logLevel, const std::string& message) const {
        const char* logPath = getLogPath(logLevel);
        std::ofstream file(logPath, std::ios::app);
        if (file.is_open()) {
            file << getCurrentTimestamp() << " " << getLevelString(logLevel) 
                 << " - " << message << std::endl;
            file.close();
        }
    }
    
    // Backward compatibility method (defaults to LOG level)
    void writeLog(const std::string& message) const {
        writeLog(LOG, message);
    }
    
    // Static instance for easy access
    static Tintin_reporter& getInstance() {
        static Tintin_reporter instance;
        return instance;
    }
    
    // Static convenience methods with log levels
    static void log(level logLevel, const std::string& message) {
        getInstance().writeLog(logLevel, message);
    }
    
    // Static convenience method (backward compatibility, defaults to LOG)
    static void log(const std::string& message) {
        getInstance().writeLog(LOG, message);
    }
};

#endif
