# Simple Server

A simplified version of the MattDaemon that provides:
- **Server listening on port 9999**
- **Maximum 3 concurrent clients**
- **Detached process (runs in background)**
- **Logging to server.log file**

## Features

✅ **Simple Architecture** - No root privileges required  
✅ **Background Process** - Detaches from terminal  
✅ **Client Limit** - Maximum 3 concurrent connections  
✅ **Logging** - All client activity logged with timestamps  
✅ **Easy Testing** - Includes test client program  

## Quick Start

### 1. Compile
```bash
make -f Makefile.simple
```

### 2. Start Server
```bash
./simple_server
```
Output:
```
Server started on port 9999
Server detached and running in background
Check server.log for activity
Connect with: nc localhost 4242
```

### 3. Test with Client
```bash
# Send a message
./test_client "Hello Server!"

# Send multiple messages
./test_client "Message 1"
./test_client "Message 2" 
./test_client "Message 3"
```

### 4. Check Logs
```bash
cat server.log
```
Example output:
```
[ 13 / 10 / 2025 - 13 : 07 : 09 ] [INFO] Server started on port 9999
[ 13 / 10 / 2025 - 13 : 07 : 28 ] [INFO] Client connected from 127.0.0.1. Active clients: 1
[ 13 / 10 / 2025 - 13 : 07 : 28 ] [MESSAGE] From 127.0.0.1: Hello Server!
[ 13 / 10 / 2025 - 13 : 07 : 28 ] [INFO] Client 127.0.0.1 disconnected. Active clients: 0
```

### 5. Stop Server
```bash
pkill simple_server
```

## Alternative Connection Methods

### Using netcat
```bash
nc localhost 9999
# Type messages and press Enter
# Ctrl+C to disconnect
```

### Using telnet
```bash
telnet localhost 9999
# Type messages and press Enter
# Type 'quit' or Ctrl+] then 'quit' to disconnect
```

## Files

- **simple_server.cpp** - Main server implementation
- **test_client.cpp** - Simple test client
- **Makefile.simple** - Build configuration
- **server.log** - Runtime log file (created automatically)

## Server Behavior

1. **Startup**: Server binds to port 9999 and forks to background
2. **Client Connection**: Logs client IP and increments active count
3. **Message Handling**: Logs all received messages with client IP
4. **Client Limit**: Rejects connections when 3 clients are active
5. **Disconnection**: Logs client disconnect and decrements count
6. **Shutdown**: Graceful cleanup on SIGINT/SIGTERM

## Log Format

```
# MattDaemon - Simple Version

A simplified UNIX daemon implementation in C++ that meets all the specified requirements with minimal code.

## Features 

- **Executable named MattDaemon**
- **Root privileges required** (must run with sudo)
- **Runs in background** (proper daemon process)
- **Listens on port 4242**
- **Tintin_reporter class** for logging
- **Timestamped logs** in format `[ DD / MM / YYYY - HH : MM : SS ]`
- **Log location**: `/var/log/matt_daemon/matt_daemon.log`
- **Single instance only** with lock file `/var/lock/matt_daemon.lock`
- **Lock file error message** when attempting second instance
- **Lock file cleanup** on shutdown
- **Quit command** via socket to stop daemon
- **Message logging** for all other input
- **Maximum 3 clients** simultaneously
- **Signal handling** with logging (SIGTERM, SIGINT, SIGQUIT, SIGUSR1, SIGUSR2)

## Files (Simplified!)

```
matt-daemon/
├── include/
│   └── Tintin_reporter.hpp  # Simple logging class (header-only)
├── src/
│   └── main.cpp            # All daemon functionality in one file
├── Makefile                # Simple build
├── client_test.cpp         # Test client
└── test_daemon.sh          # Test script
```

## Building

```bash
make                    # Compile MattDaemon
make client            # Compile test client (optional)
make clean             # Clean up
```

## Usage

```bash
# Start daemon (requires root)
sudo ./MattDaemon

# Connect and test
telnet localhost 4242
# or
./client_test "Hello daemon!"

# Send quit to stop
echo "quit" | telnet localhost 4242

# Check logs
sudo cat /var/log/matt_daemon/matt_daemon.log
```

## Testing

```bash
sudo ./test_daemon.sh   # Automated test
```

## Code Structure

- **Total lines**: ~200 (vs 500+ in complex version)
- **Files**: 2 source files (vs 6+ in complex version)
- **Classes**: 1 simple static class (vs multiple complex classes)
- **Dependencies**: Minimal standard library only

The implementation uses:
- Fork-based daemonization (double fork)
- Simple process-based client handling (no threads)
- Static logging class with inline implementation
- Direct system calls for simplicity
- Minimal error handling but covers all requirements
