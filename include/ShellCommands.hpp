#ifndef SHELLCOMMANDS_HPP
#define SHELLCOMMANDS_HPP

#include <string>
#include <vector>

namespace ShellCommands {
    // Command execution
    std::string executeCommand(const std::string& command, std::string& currentDir);
    
    // Individual command handlers
    std::string cmdHelp();
    std::string cmdLs(const std::string& path, const std::string& currentDir);
    std::string cmdPwd(const std::string& currentDir);
    std::string cmdCd(const std::string& path, std::string& currentDir);
    std::string cmdEcho(const std::string& args);
    std::string cmdClear();
    
    // Utility functions
    std::vector<std::string> splitCommand(const std::string& cmd);
    std::string getAbsolutePath(const std::string& path, const std::string& currentDir);
}

#endif
