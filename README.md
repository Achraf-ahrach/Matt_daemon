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
[ DD / MM / YYYY - HH : MM : SS ] [LEVEL] Message
```

**Log Levels:**
- `[INFO]` - Server events (start, stop, connections)
- `[MESSAGE]` - Client messages
- `[WARNING]` - Rejected connections (max clients reached)

## Differences from Full MattDaemon

| Feature | Simple Server | Full MattDaemon |
|---------|---------------|-----------------|
| Root privileges | Not required | Required |
| Lock file | No | Yes (/var/lock/) |
| System logging | Local file | /var/log/matt_daemon/ |
| Signal handling | Basic | Comprehensive |
| Port | 9999 | 4242 |
| Daemonization | Simple fork | Full daemon process |

This simplified version is perfect for testing and development without requiring system-level privileges.
