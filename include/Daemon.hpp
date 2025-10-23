#ifndef DAEMON_HPP
#define DAEMON_HPP

namespace Daemon {
    void daemonize();
    void setupSignalHandlers();
    void signalHandler(int sig);
}

extern bool running;

#endif