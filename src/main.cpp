#include "MattDaemon.hpp"
#include <iostream>

int main() {
    MattDaemon daemon;
    
    if (!daemon.initialize()) {
        std::cerr << "Failed to initialize MattDaemon" << std::endl;
        return 1;
    }
    
    daemon.run();
    
    return 0;
}