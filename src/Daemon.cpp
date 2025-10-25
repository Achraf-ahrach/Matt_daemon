#include "Daemon.hpp"
#include "Tintin_reporter.hpp"
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>

bool running = true;

void Daemon::signalHandler(int sig) {
    std::string sigName;
    switch(sig) {
        case SIGHUP: sigName = "SIGHUP"; break;
        case SIGINT: sigName = "SIGINT"; break;
        case SIGQUIT: sigName = "SIGQUIT"; break;
        case SIGILL: sigName = "SIGILL"; break;
        case SIGTRAP: sigName = "SIGTRAP"; break;
        case SIGABRT: sigName = "SIGABRT"; break;
        case SIGFPE: sigName = "SIGFPE"; break;
        case SIGBUS: sigName = "SIGBUS"; break;
        case SIGSEGV: sigName = "SIGSEGV"; break;
        case SIGSYS: sigName = "SIGSYS"; break;
        case SIGPIPE: sigName = "SIGPIPE"; break;
        case SIGALRM: sigName = "SIGALRM"; break;
        case SIGTERM: sigName = "SIGTERM"; break;
        case SIGURG: sigName = "SIGURG"; break;
        case SIGTSTP: sigName = "SIGTSTP"; break;
        case SIGCONT: sigName = "SIGCONT"; break;
        case SIGCHLD: sigName = "SIGCHLD"; break;
        case SIGTTIN: sigName = "SIGTTIN"; break;
        case SIGTTOU: sigName = "SIGTTOU"; break;
        case SIGIO: sigName = "SIGIO"; break;
        case SIGXCPU: sigName = "SIGXCPU"; break;
        case SIGXFSZ: sigName = "SIGXFSZ"; break;
        case SIGVTALRM: sigName = "SIGVTALRM"; break;
        case SIGPROF: sigName = "SIGPROF"; break;
        case SIGWINCH: sigName = "SIGWINCH"; break;
        case SIGUSR1: sigName = "SIGUSR1"; break;
        case SIGUSR2: sigName = "SIGUSR2"; break;
        default: sigName = "UNKNOWN"; break;
    }
    
    Tintin_reporter::log(INFO, "Matt_daemon: Signal " + sigName + " received.");
    running = false;
}

void Daemon::setupSignalHandlers() {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
}

void Daemon::daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    chdir("/");
    umask(0);
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
}