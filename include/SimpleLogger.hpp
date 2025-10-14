#ifndef SIMPLELOGGER_HPP
#define SIMPLELOGGER_HPP

#include <string>

class SimpleLogger {
private:
    std::string logFile;
    
    std::string getCurrentTimestamp() const;
    
public:
    SimpleLogger(const std::string& file);
    
    void log(const std::string& message);
};

#endif
