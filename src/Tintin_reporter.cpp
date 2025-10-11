#include "Tintin_reporter.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

Tintin_reporter::Tintin_reporter(const std::string& logFile) : _logFile(logFile) {
    ensureLogDirectory();
    _fileStream.open(_logFile, std::ios::app);
    if (_fileStream.is_open()) {
        log("Tintin_reporter started");
    }
}

Tintin_reporter::~Tintin_reporter() {
    if (_fileStream.is_open()) {
        log("Tintin_reporter stopped");
        _fileStream.close();
    }
}

std::string Tintin_reporter::getCurrentTimestamp() const {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    
    std::ostringstream oss;
    oss << "[ " << std::setfill('0') << std::setw(2) << localTime->tm_mday
        << " / " << std::setw(2) << (localTime->tm_mon + 1)
        << " / " << (localTime->tm_year + 1900)
        << " - " << std::setw(2) << localTime->tm_hour
        << " : " << std::setw(2) << localTime->tm_min
        << " : " << std::setw(2) << localTime->tm_sec << " ]";
    
    return oss.str();
}

void Tintin_reporter::ensureLogDirectory() const {
    std::string logDir = "/var/log/matt_daemon";
    struct stat st;
    memset(&st, 0, sizeof(st));
    
    if (stat(logDir.c_str(), &st) == -1) {
        if (mkdir(logDir.c_str(), 0755) != 0) {
            std::cerr << "Error: Cannot create log directory " << logDir << std::endl;
        }
    }
}

void Tintin_reporter::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(_logMutex);
    if (_fileStream.is_open()) {
        _fileStream << getCurrentTimestamp() << " " << message << std::endl;
        _fileStream.flush();
    }
}

void Tintin_reporter::logInfo(const std::string& message) {
    log("[INFO] " + message);
}

void Tintin_reporter::logError(const std::string& message) {
    log("[ERROR] " + message);
}

void Tintin_reporter::logWarning(const std::string& message) {
    log("[WARNING] " + message);
}

bool Tintin_reporter::isOpen() const {
    return _fileStream.is_open();
}
