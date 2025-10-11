#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <string>
#include <fstream>
#include <mutex>

class Tintin_reporter {
private:
    std::string _logFile;
    std::ofstream _fileStream;
    std::mutex _logMutex;
    
    std::string getCurrentTimestamp() const;
    void ensureLogDirectory() const;

public:
    Tintin_reporter(const std::string& logFile);
    ~Tintin_reporter();
    
    void log(const std::string& message);
    void logInfo(const std::string& message);
    void logError(const std::string& message);
    void logWarning(const std::string& message);
    
    bool isOpen() const;
};

#endif
