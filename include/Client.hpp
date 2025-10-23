#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

namespace Client {
    void setupSignalHandlers();
    void signalHandler(int sig);
    void handleClient(int clientSock, const std::string& clientInfo);
}

#endif