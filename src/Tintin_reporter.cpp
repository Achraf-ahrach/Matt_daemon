#include "Tintin_reporter.hpp"

std::string Tintin_reporter::getCurrentTimestamp() const {
    std::time_t now = std::time(nullptr);
    now += 3600; // add +1 hour (sa3a jdida)

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

void Tintin_reporter::ensureLogDirectory() const {
    mkdir(BASE_PATH, 0755);
}

const char* Tintin_reporter::getLogPath(level logLevel) const {
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

std::string Tintin_reporter::getLevelString(level logLevel) const {
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

Tintin_reporter::Tintin_reporter() {
    ensureLogDirectory();
}

Tintin_reporter::Tintin_reporter(const Tintin_reporter& other) {
    (void)other;
}

Tintin_reporter& Tintin_reporter::operator=(const Tintin_reporter& other) {
    (void)other;
    return *this;
}

Tintin_reporter::~Tintin_reporter() {}

void Tintin_reporter::writeLog(level logLevel, const std::string& message) const {
    const char* logPath = getLogPath(logLevel);
    std::ofstream file(logPath, std::ios::app);
    if (file.is_open()) {
        file << getCurrentTimestamp() << " " << getLevelString(logLevel) 
             << " - " << message << std::endl;
        file.close();
    }
}

void Tintin_reporter::writeLog(const std::string& message) const {
    writeLog(LOG, message);
}

Tintin_reporter& Tintin_reporter::getInstance() {
    static Tintin_reporter instance;
    return instance;
}

void Tintin_reporter::log(level logLevel, const std::string& message) {
    getInstance().writeLog(logLevel, message);
}

void Tintin_reporter::log(const std::string& message) {
    getInstance().writeLog(LOG, message);
}