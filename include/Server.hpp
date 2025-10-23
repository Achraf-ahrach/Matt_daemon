#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>

class Server {
private:
    int serverSocket;
    int lockFd;
    int activeClients;
    
public:
    Server();
    ~Server();
    
    bool initialize();
    void run();
    int getSocket() const { return serverSocket; }
    int getLockFd() const { return lockFd; }
};

#endif