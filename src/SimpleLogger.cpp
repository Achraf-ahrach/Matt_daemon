#include "SimpleLogger.hpp"
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

SimpleLogger::SimpleLogger(const std::string& file) : logFile(file) {}

std::string SimpleLogger::getCurrentTimestamp() const {
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

void SimpleLogger::log(const std::string& message) {
    std::ofstream file(logFile, std::ios::app);
    if (file.is_open()) {
        file << getCurrentTimestamp() << " " << message << std::endl;
        file.close();
    }
}
