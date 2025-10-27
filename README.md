# Matt_daemon 🔒

A robust UNIX daemon implementation in C++ featuring TCP server capabilities, authentication, shell commands, and a Qt5 GUI client. Built with security and reliability in mind.

[![C++11](https://img.shields.io/badge/C++-11-blue.svg)](https://isocpp.org/)
[![Qt5](https://img.shields.io/badge/Qt-5-green.svg)](https://www.qt.io/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

## 📋 Table of Contents

- [Features](#-features)
- [Architecture](#-architecture)
- [Requirements](#-requirements)
- [Installation](#-installation)
- [Usage](#-usage)
- [Configuration](#-configuration)
- [Logging](#-logging)
- [Security](#-security)
- [GUI Client](#-gui-client)
- [Shell Commands](#-shell-commands)
- [Troubleshooting](#-troubleshooting)
- [Project Structure](#-project-structure)

## ✨ Features

### Core Daemon Functionality

- ✅ **Proper Daemonization** - Double fork, session leader, directory/umask management
- ✅ **Single Instance Enforcement** - Lock file mechanism prevents multiple instances
- ✅ **Root Privilege Requirement** - Enhanced security with privilege checking
- ✅ **Comprehensive Signal Handling** - Graceful shutdown for 27+ different signals
- ✅ **Background Execution** - Detaches from terminal and runs as true daemon

### Networking

- 🌐 **TCP Server on Port 4242** - Reliable socket-based communication
- 👥 **Multi-Client Support** - Handles up to 3 concurrent clients
- 🔐 **Authentication System** - Username/password protection
- 🚫 **Connection Limiting** - Automatic rejection when max clients reached

### Logging System

- 📝 **Tintin_reporter** - Singleton logger with timestamped entries
- 🗂️ **Multi-Level Logging** - ERROR, LOG, INFO severity levels
- 📁 **Organized Log Files** - Separate files per level + main log
- ⏰ **Timestamp Format** - `[ DD / MM / YYYY - HH : MM : SS ]`
- 📍 **Centralized Location** - `/var/log/matt_daemon/`

### Shell Commands

- 💻 **Built-in Commands** - help, ls, pwd, cd, echo, clear, quit
- 📂 **Directory Navigation** - Full path resolution and management
- 🎨 **Colored Prompt** - Enhanced terminal experience
- ⚡ **Real-time Execution** - Immediate command processing

### Notifications

- 📧 **Email Alerts** - Gmail SMTP notifications on client disconnect
- 📊 **Detailed Reports** - Timestamp, PID, client info in emails

### GUI Client

- 🖥️ **Qt5-Based Interface** - Modern, native cross-platform GUI
- 🔌 **Easy Connection** - One-click server connection
- 📜 **Command History** - Scrollable log display
- 🎨 **Professional Design** - Clean, user-friendly interface

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────┐
│              Matt_daemon Process                 │
│  ┌──────────────────────────────────────────┐  │
│  │         Daemon Core (main.cpp)           │  │
│  │  - Root check                            │  │
│  │  - Lock file creation                    │  │
│  │  - Daemonization (double fork)           │  │
│  │  - Signal handler setup                  │  │
│  └──────────────────────────────────────────┘  │
│                      ▼                          │
│  ┌──────────────────────────────────────────┐  │
│  │      TCP Server (Server.cpp)             │  │
│  │  - Bind to port 4242                     │  │
│  │  - Listen for connections                │  │
│  │  - Client limit enforcement              │  │
│  └──────────────────────────────────────────┘  │
│                      ▼                          │
│  ┌──────────────────────────────────────────┐  │
│  │    Client Handler (Client.cpp)           │  │
│  │  - Authentication (Auth.cpp)             │  │
│  │  - Shell commands (ShellCommands.cpp)    │  │
│  │  - Per-client process                    │  │
│  └──────────────────────────────────────────┘  │
│                      ▼                          │
│  ┌──────────────────────────────────────────┐  │
│  │    Logging (Tintin_reporter.cpp)         │  │
│  │  - Singleton pattern                     │  │
│  │  - File I/O to /var/log/matt_daemon/     │  │
│  └──────────────────────────────────────────┘  │
└─────────────────────────────────────────────────┘

         ▲                            ▲
         │                            │
    ┌────┴────┐                  ┌────┴────┐
    │ Clients │                  │   GUI   │
    │(telnet, │                  │ Client  │
    │  nc)    │                  │ (Qt5)   │
    └─────────┘                  └─────────┘
```

## 📦 Requirements

### System Requirements

- **Operating System**: Linux or macOS
- **Compiler**: g++ or clang with C++11 support
- **Privileges**: Root access for daemon execution

### Dependencies

**For Daemon:**

- Standard C++ Library
- POSIX compliant system (unistd.h, sys/socket.h, etc.)

**For GUI Client:**

- Qt5 (QtCore, QtWidgets, QtNetwork)
- Qt5 MOC (Meta-Object Compiler)

### macOS Installation (Homebrew)

```bash
# Install Qt5
brew install qt@5

# Add to PATH
echo 'export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### Linux Installation

```bash
# Debian/Ubuntu
sudo apt-get install qt5-default qtbase5-dev

# Fedora/RHEL
sudo dnf install qt5-qtbase-devel

# Arch Linux
sudo pacman -S qt5-base
```

## 🚀 Installation

### 1. Clone Repository

```bash
git clone https://github.com/yourusername/Matt_daemon.git
cd Matt_daemon
```

### 2. Build Daemon

```bash
make
```

### 3. Build GUI Client (Optional)

```bash
make client
# or
make gui
```

### 4. Clean Build

```bash
make clean   # Remove object files
make fclean  # Remove everything
make re      # Rebuild from scratch
```

## 💻 Usage

### Starting the Daemon

```bash
# Must run with root privileges
sudo ./MattDaemon
```

**Expected Output:**

```
Matt_daemon: Started.
Matt_daemon: Entering Daemon mode.
Matt_daemon: started. PID: 12345.
Matt_daemon: Server created.
```

### Checking if Daemon is Running

```bash
# Check process
ps aux | grep MattDaemon

# Check lock file
cat /var/lock/matt_daemon.lock

# Check logs
sudo tail -f /var/log/matt_daemon/matt_daemon.log
```

### Connecting with Terminal Client

```bash
# Using telnet
telnet localhost 4242

# Using netcat
nc localhost 4242
```

**Login Process:**

```
========================================
  Matt_daemon Remote Shell
========================================
Please login to continue.
========================================

Username: admin
Password: admin

✓ Login successful!

========================================
Type 'help' for available commands.
========================================

matt_daemon:/$
```

### Using GUI Client

```bash
./matt_daemon_client
```

1. Click **Connect** button
2. Auto-authentication with default credentials
3. Enter commands in the input field
4. Click **Send** or press Enter

### Stopping the Daemon

```bash
# Send SIGTERM
sudo pkill MattDaemon

# Or send SIGINT
sudo killall -INT MattDaemon

# Check logs for graceful shutdown
sudo tail /var/log/matt_daemon/matt_daemon.log
```

## ⚙️ Configuration

### Lock File Location

```cpp
// include/Utils.hpp
#define LOCK_FILE "/var/lock/matt_daemon.lock"
```

### Server Port

```cpp
// include/Utils.hpp
#define PORT 4242
```

### Maximum Clients

```cpp
// include/Utils.hpp
#define MAX_CLIENTS 3
```

### Authentication Credentials

```cpp
// include/Auth.hpp
static constexpr const char* ADMIN_USERNAME = "admin";
static constexpr const char* ADMIN_PASSWORD = "admin";
```

⚠️ **Security Warning**: Change default credentials in production!

### Email Notifications

```cpp
// include/Utils.hpp
#define EMAIL_FROM "your-email@gmail.com"
#define EMAIL_TO   "recipient@gmail.com"
#define EMAIL_PASS "your-app-password"
```

**Gmail App Password Setup:**

1. Enable 2-Factor Authentication on your Gmail account
2. Go to Google Account → Security → App passwords
3. Generate new app password
4. Use 16-character password in `EMAIL_PASS`

## 📊 Logging

### Log Files

| File                                     | Purpose               | Severity |
| ---------------------------------------- | --------------------- | -------- |
| `/var/log/matt_daemon/matt_daemon.log` | Main log (all levels) | ALL      |
| `/var/log/matt_daemon/Error.log`       | Error messages only   | ERROR    |
| `/var/log/matt_daemon/Log.log`         | General logs          | LOG      |
| `/var/log/matt_daemon/Info.log`        | Info messages         | INFO     |

### Log Format

```
[ DD / MM / YYYY - HH : MM : SS ] [ LEVEL ] - Message
```

**Example:**

```
[ 13 / 10 / 2025 - 14 : 30 : 45 ] [ INFO ] - Matt_daemon: Started.
[ 13 / 10 / 2025 - 14 : 30 : 45 ] [ INFO ] - Matt_daemon: Entering Daemon mode.
[ 13 / 10 / 2025 - 14 : 30 : 47 ] [ INFO ] - Matt_daemon: started. PID: 12345.
[ 13 / 10 / 2025 - 14 : 31 : 12 ] [ INFO ] - Matt_daemon: Client connected from 127.0.0.1:54321
[ 13 / 10 / 2025 - 14 : 31 : 15 ] [ LOG ] - Matt_daemon: User input: ls
[ 13 / 10 / 2025 - 14 : 35 : 20 ] [ INFO ] - Matt_daemon: Signal SIGTERM received.
```

### Viewing Logs

```bash
# Tail all logs
sudo tail -f /var/log/matt_daemon/matt_daemon.log

# View errors only
sudo cat /var/log/matt_daemon/Error.log

# Search logs
sudo grep "Client connected" /var/log/matt_daemon/matt_daemon.log

# Last 50 lines
sudo tail -n 50 /var/log/matt_daemon/matt_daemon.log
```

## 🔒 Security

### Authentication System

- **3 Login Attempts Maximum** - Account lockout after failed attempts
- **Credential Validation** - Username and password verification
- **Session-Based** - Per-client authentication state

### Process Isolation

- **Fork-Based Client Handling** - Each client runs in separate process
- **Signal Isolation** - Independent signal handlers per process
- **Resource Cleanup** - Automatic cleanup on client disconnect

### File Permissions

```bash
# Lock file
-rw-r--r-- 1 root root /var/lock/matt_daemon.lock

# Log directory
drwxr-xr-x 2 root root /var/log/matt_daemon/

# Log files
-rw-r--r-- 1 root root /var/log/matt_daemon/*.log
```

### Signal Handling

Gracefully handles these signals:

- `SIGTERM`, `SIGINT`, `SIGQUIT` - Graceful shutdown
- `SIGHUP` - Client disconnect detection
- `SIGPIPE` - Broken pipe handling
- And 22+ additional signals

## 🖥️ GUI Client

### Features

- **Connection Management** - Connect/Disconnect buttons
- **Live Log Display** - Real-time command output
- **Command Input** - Text field with Enter key support
- **Status Bar** - Connection status indicator
- **Timestamps** - All messages timestamped
- **Auto-scroll** - Automatically scrolls to latest message

### Screenshots

```
┌─────────────────────────────────────────────────────────┐
│  MattDaemon Client                                  ─ □ ×│
├─────────────────────────────────────────────────────────┤
│ [Log Display Area]                                      │
│ [2025-01-13 14:30:45] MattDaemon Client started...      │
│ [2025-01-13 14:30:50] Connecting to server...           │
│ [2025-01-13 14:30:51] Connected to MattDaemon server.   │
│ [2025-01-13 14:30:52] << Username:                      │
│ [2025-01-13 14:30:52] >> admin                          │
│ [2025-01-13 14:30:52] << Password:                      │
│ [2025-01-13 14:30:52] >> ****                           │
│ [2025-01-13 14:30:53] << ✓ Login successful!            │
│                                                          │
├─────────────────────────────────────────────────────────┤
│ [Connect] [Disconnect]                                  │
│ [Command: _________________________] [Send]             │
├─────────────────────────────────────────────────────────┤
│ Connected to MattDaemon server                          │
└─────────────────────────────────────────────────────────┘
```

### Keyboard Shortcuts

- **Enter** - Send command
- **Ctrl+C** - Copy selected text
- **Ctrl+A** - Select all in log display

## 🛠️ Shell Commands

| Command         | Description                | Example                          |
| --------------- | -------------------------- | -------------------------------- |
| `help`        | Display available commands | `help`                         |
| `ls [path]`   | List directory contents    | `ls /var`, `ls`              |
| `pwd`         | Print working directory    | `pwd`                          |
| `cd <path>`   | Change directory           | `cd /tmp`, `cd ..`, `cd ~` |
| `echo <text>` | Display text               | `echo Hello World`             |
| `clear`       | Clear screen               | `clear`                        |
| `quit`        | Exit client session        | `quit`                         |

### Command Examples

```bash
matt_daemon:/$ help

=== Matt_daemon Remote Shell - Available Commands ===

  help                 - Display this help message
  ls [path]            - List directory contents
  pwd                  - Print working directory
  cd <path>            - Change directory
  echo <text>          - Display a line of text
  clear                - Clear the screen
  quit                 - Exit the client connection

Note: All commands are executed in the daemon's context.
======================================================

matt_daemon:/$ pwd
/

matt_daemon:/$ cd /var/log
matt_daemon:/var/log$ ls
daemon.log
matt_daemon/
system.log

matt_daemon:/var/log$ cd matt_daemon
matt_daemon:/var/log/matt_daemon$ ls
Error.log
Info.log
Log.log
matt_daemon.log

matt_daemon:/var/log/matt_daemon$ echo "Test message"
Test message

matt_daemon:/var/log/matt_daemon$ quit
Goodbye!
```

## 🐛 Troubleshooting

### Daemon Won't Start

**Problem:** `Error: Matt_daemon instance already running`

**Solution:**

```bash
# Check if process is running
ps aux | grep MattDaemon

# If not running but lock file exists, remove it
sudo rm /var/lock/matt_daemon.lock

# Try again
sudo ./MattDaemon
```

---

**Problem:** `Error: Matt_daemon must be run with root privileges`

**Solution:**

```bash
# Run with sudo
sudo ./MattDaemon
```

---

**Problem:** `Cannot bind to port 4242`

**Solution:**

```bash
# Check what's using the port
sudo lsof -i :4242

# Kill the process if needed
sudo kill -9 <PID>

# Or change PORT in include/Utils.hpp and recompile
```

### Client Connection Issues

**Problem:** `Connection refused`

**Solution:**

```bash
# Verify daemon is running
ps aux | grep MattDaemon

# Check if port is listening
sudo lsof -i :4242

# Check firewall (macOS)
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --getglobalstate

# Check firewall (Linux)
sudo ufw status
```

---

**Problem:** `Server full: Maximum 3 clients allowed`

**Solution:**

- Wait for a client to disconnect
- Or increase `MAX_CLIENTS` in [`include/Utils.hpp`](include/Utils.hpp) and recompile

### GUI Client Issues

**Problem:** Qt5 not found during compilation

**Solution:**

```bash
# macOS
brew install qt@5
export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"

# Linux (Debian/Ubuntu)
sudo apt-get install qt5-default qtbase5-dev

# Update Makefile QT5_PATH if needed
```

---

**Problem:** `MOC not found`

**Solution:**

```bash
# Check MOC location
which moc

# Update Makefile MOC variable
# Example: MOC = /usr/bin/moc
```

### Log Issues

**Problem:** No logs appearing

**Solution:**

```bash
# Check directory exists
sudo ls -la /var/log/matt_daemon/

# Create if missing
sudo mkdir -p /var/log/matt_daemon

# Check permissions
sudo chmod 755 /var/log/matt_daemon

# Restart daemon
sudo pkill MattDaemon
sudo ./MattDaemon
```

## 📁 Project Structure

```
Matt_daemon/
├── include/                      # Header files
│   ├── Auth.hpp                  # Authentication class
│   ├── Client.hpp                # Client handler declarations
│   ├── Daemon.hpp                # Daemon core functions
│   ├── GUI.hpp                   # Qt5 GUI class
│   ├── Server.hpp                # TCP server class
│   ├── ShellCommands.hpp         # Built-in shell commands
│   ├── Tintin_reporter.hpp       # Logging system
│   └── Utils.hpp                 # Utility functions & constants
│
├── src/                          # Source files
│   ├── Auth.cpp                  # Authentication implementation
│   ├── Client.cpp                # Client connection handling
│   ├── Daemon.cpp                # Daemonization & signals
│   ├── GUI.cpp                   # Qt5 GUI implementation
│   ├── gui_main.cpp              # GUI entry point
│   ├── main.cpp                  # Daemon entry point
│   ├── Server.cpp                # TCP server implementation
│   ├── ShellCommands.cpp         # Command execution logic
│   ├── Tintin_reporter.cpp       # Logging implementation
│   └── Utils.cpp                 # Utility functions
│
├── obj/                          # Object files (generated)
│   ├── *.o                       # Compiled object files
│   └── moc_GUI.cpp               # Qt MOC generated file
│
├── Makefile                      # Build configuration
├── README.md                     # This file
└── .gitignore                    # Git ignore rules
```

### Component Descriptions

| Component                | File                                            | Responsibility                                              |
| ------------------------ | ----------------------------------------------- | ----------------------------------------------------------- |
| **Entry Point**    | [`main.cpp`](src/main.cpp)                       | Root check, lock file, daemonization, server initialization |
| **Daemon Core**    | [`Daemon.cpp`](src/Daemon.cpp)                   | Double fork, signal handling, background execution          |
| **TCP Server**     | [`Server.cpp`](src/Server.cpp)                   | Socket creation, binding, listening, client limit           |
| **Client Handler** | [`Client.cpp`](src/Client.cpp)                   | Authentication, command processing, per-client process      |
| **Authentication** | [`Auth.cpp`](src/Auth.cpp)                       | Credential validation, thread-safe login                    |
| **Shell Commands** | [`ShellCommands.cpp`](src/ShellCommands.cpp)     | Command parsing, execution, directory management            |
| **Logging**        | [`Tintin_reporter.cpp`](src/Tintin_reporter.cpp) | Singleton logger, timestamped logs, file I/O                |
| **Utilities**      | [`Utils.cpp`](src/Utils.cpp)                     | Lock file, email notifications, cleanup                     |
| **GUI Client**     | [`GUI.cpp`](src/GUI.cpp)                         | Qt5 interface, socket communication, display                |

## 🔄 Process Flow

### Daemon Startup

```
1. main() checks for root privileges
2. Create lock file at /var/lock/matt_daemon.lock
3. Initialize TCP server on port 4242
4. Call Daemon::daemonize()
   - First fork() → parent exits
   - setsid() → become session leader
   - Second fork() → parent exits
   - chdir("/") → change to root directory
   - umask(0) → clear file creation mask
   - Close stdin/stdout/stderr
5. Setup signal handlers for 27 signals
6. Enter server.run() loop
```

### Client Connection

```
1. accept() new client connection
2. Check if activeClients < MAX_CLIENTS
3. fork() new process for client
4. Child process:
   - Close server socket
   - Send login banner
   - Request username/password (3 attempts)
   - Authenticate via Auth class
   - If successful: enter command loop
   - If failed: disconnect
5. Parent process:
   - Close client socket
   - Increment activeClients
   - Continue accepting connections
```

### Command Execution

```
1. Client sends command
2. Log command via Tintin_reporter
3. Parse command in ShellCommands::executeCommand()
4. Execute corresponding function:
   - help → cmdHelp()
   - ls → cmdLs()
   - pwd → cmdPwd()
   - cd → cmdCd() (updates currentDir)
   - echo → cmdEcho()
   - clear → cmdClear()
   - quit → exit connection
5. Send response to client
6. Send new prompt
```

## 📝 Development

### Building from Source

```bash
# Full build
make

# Build daemon only
make all

# Build GUI client only
make client

# Parallel build (faster)
make -j4

# Verbose build
make VERBOSE=1
```

### Code Style

- **Standard**: C++11
- **Indentation**: Tabs (as per Makefile)
- **Naming**:
  - Classes: `PascalCase`
  - Functions: `camelCase`
  - Constants: `UPPER_SNAKE_CASE`
  - Files: match class names

### Adding New Commands

1. Declare in [`include/ShellCommands.hpp`](include/ShellCommands.hpp):

```cpp
std::string cmdYourCommand(const std::string& args);
```

2. Implement in [`src/ShellCommands.cpp`](src/ShellCommands.cpp):

```cpp
std::string cmdYourCommand(const std::string& args) {
    // Your implementation
    return result;
}
```

3. Add to `executeCommand()`:

```cpp
else if (cmd == "yourcommand") {
    return cmdYourCommand(args);
}
```

4. Update `cmdHelp()` documentation

## 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open Pull Request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 👥 Authors

- **Achraf Ahrach -** [Achraf-ahrach](https://github.com/Achraf-ahrach)
- **Abderrahman el asri** **- [abde-r](https://github.com/abde-r)**

## 🙏 Acknowledgments

- Inspired by traditional UNIX daemon implementations
- Qt5 for the excellent GUI framework
- Gmail SMTP for notification support

## 📞 Support

For issues, questions, or contributions:

- Open an issue on GitHub
- Email: achrafahrach44@gmail.com

---

**Made with ❤️ by the Matt_daemon team**
