
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

enum level {
    ERROR,
    LOG,
    INFO
};

#define BASE_PATH  "/var/log/matt_daemon"
#define ERROR_PATH  "/var/log/matt_daemon/Error.log"
#define LOG_PATH  "/var/log/matt_daemon/Log.log"
#define INFO_PATH  "/var/log/matt_daemon/Info.log"
#define MAIN_LOG_PATH  "/var/log/matt_daemon/matt_daemon.log"

class Tintin_reporter {
private:
    std::string getCurrentTimestamp() const;
    void ensureLogDirectory() const;
    const char* getLogPath(level logLevel) const;
    std::string getLevelString(level logLevel) const;

public:
    Tintin_reporter();
    Tintin_reporter(const Tintin_reporter& other);
    Tintin_reporter& operator=(const Tintin_reporter& other);
    ~Tintin_reporter();
    
    void writeLog(level logLevel, const std::string& message) const;
    void writeLog(const std::string& message) const;
    
    static Tintin_reporter& getInstance();
    static void log(level logLevel, const std::string& message);
    static void log(const std::string& message);
};

#endif