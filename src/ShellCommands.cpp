#include "ShellCommands.hpp"
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <climits>

extern char** environ;

namespace ShellCommands {

std::vector<std::string> splitCommand(const std::string& cmd) {
    std::vector<std::string> tokens;
    std::stringstream ss(cmd);
    std::string token;
    
    while (ss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string getAbsolutePath(const std::string& path, const std::string& currentDir) {
    if (path.empty() || path[0] == '/') {
        return path;
    }
    
    if (path == ".") {
        return currentDir;
    }
    
    if (path == "..") {
        size_t pos = currentDir.find_last_of('/');
        if (pos != std::string::npos && pos > 0) {
            return currentDir.substr(0, pos);
        }
        return "/";
    }
    
    std::string result = currentDir;
    if (result.back() != '/') {
        result += "/";
    }
    result += path;
    
    return result;
}

std::string cmdHelp() {
    std::stringstream ss;
    ss << "\n=== Matt_daemon Remote Shell - Available Commands ===\n\n";
    ss << "  help                 - Display this help message\n";
    ss << "  ls [path]            - List directory contents\n";
    ss << "  pwd                  - Print working directory\n";
    ss << "  cd <path>            - Change directory\n";
    ss << "  echo <text>          - Display a line of text\n";
    ss << "  clear                - Clear the screen\n";
    ss << "  quit                 - Exit the client connection\n\n";
    ss << "Note: All commands are executed in the daemon's context.\n";
    ss << "======================================================\n";
    return ss.str();
}

std::string cmdLs(const std::string& path, const std::string& currentDir) {
    std::string targetPath = path.empty() ? currentDir : getAbsolutePath(path, currentDir);
    
    DIR* dir = opendir(targetPath.c_str());
    if (!dir) {
        return "Error: Cannot open directory '" + targetPath + "'\n";
    }
    
    std::stringstream ss;
    std::vector<std::string> entries;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name != "." && name != "..") {
            std::string fullPath = targetPath;
            if (fullPath.back() != '/') fullPath += "/";
            fullPath += name;
            
            struct stat st;
            if (stat(fullPath.c_str(), &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    entries.push_back(name + "/");
                } else {
                    entries.push_back(name);
                }
            } else {
                entries.push_back(name);
            }
        }
    }
    closedir(dir);
    
    std::sort(entries.begin(), entries.end());
    
    for (const auto& e : entries) {
        ss << e << "\n";
    }
    
    return ss.str();
}

std::string cmdPwd(const std::string& currentDir) {
    return currentDir + "\n";
}

std::string cmdCd(const std::string& path, std::string& currentDir) {
    std::string targetPath;
    
    if (path.empty() || path == "~") {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            targetPath = pw->pw_dir;
        } else {
            return "Error: Cannot determine home directory\n";
        }
    } else {
        targetPath = getAbsolutePath(path, currentDir);
    }
    
    // Resolve path (handle .. and .)
    char resolved[PATH_MAX];
    if (realpath(targetPath.c_str(), resolved) == NULL) {
        return "Error: Cannot access '" + path + "': No such file or directory\n";
    }
    
    struct stat st;
    if (stat(resolved, &st) != 0 || !S_ISDIR(st.st_mode)) {
        return "Error: '" + path + "' is not a directory\n";
    }
    
    currentDir = resolved;
    return "";
}

std::string cmdEcho(const std::string& args) {
    return args + "\n";
}

std::string cmdClear() {
    return "\033[2J\033[H";
}

std::string executeCommand(const std::string& command, std::string& currentDir) {
    std::vector<std::string> tokens = splitCommand(command);
    
    if (tokens.empty()) {
        return "";
    }
    
    std::string cmd = tokens[0];
    std::string args;
    
    // Reconstruct arguments
    if (tokens.size() > 1) {
        size_t firstSpace = command.find(' ');
        if (firstSpace != std::string::npos) {
            args = command.substr(firstSpace + 1);
            // Trim leading spaces
            size_t start = args.find_first_not_of(" \t");
            if (start != std::string::npos) {
                args = args.substr(start);
            }
        }
    }
    
    if (cmd == "help" || cmd == "?") {
        return cmdHelp();
    } else if (cmd == "ls") {
        return cmdLs(tokens.size() > 1 ? tokens[1] : "", currentDir);
    } else if (cmd == "pwd") {
        return cmdPwd(currentDir);
    } else if (cmd == "cd") {
        return cmdCd(tokens.size() > 1 ? tokens[1] : "", currentDir);
    } else if (cmd == "echo") {
        return cmdEcho(args);
    } else if (cmd == "clear") {
        return cmdClear();
    } else {
        return "Error: Command not found: '" + cmd + "'. Type 'help' for available commands.\n";
    }
}

} // namespace ShellCommands
