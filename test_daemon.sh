#!/bin/bash

# Test script for MattDaemon

echo "🧪 MattDaemon Test Script"
echo "========================="

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "❌ This script must be run as root (use sudo)"
    exit 1
fi

# Function to cleanup
cleanup() {
    echo "🧹 Cleaning up..."
    pkill -TERM MattDaemon 2>/dev/null
    rm -f /tmp/matt_daemon.lock 2>/dev/null
    sleep 2
}

# Cleanup any existing instances
cleanup

echo "📦 Starting MattDaemon..."
./MattDaemon &
DAEMON_PID=$!

# Wait for daemon to start
sleep 2

# Check if daemon is running
if ! kill -0 $DAEMON_PID 2>/dev/null; then
    echo "❌ Failed to start daemon"
    exit 1
fi

echo "✅ Daemon started with PID: $DAEMON_PID"

# Check lock file
if [ -f "/tmp/matt_daemon.lock" ]; then
    echo "✅ Lock file created: /tmp/matt_daemon.lock"
    echo "   Content: $(cat /tmp/matt_daemon.lock)"
else
    echo "❌ Lock file not found"
fi

# Check log file
if [ -f "/var/log/matt_daemon/matt_daemon.log" ]; then
    echo "✅ Log file created: /var/log/matt_daemon/matt_daemon.log"
    echo "   Recent entries:"
    tail -5 /var/log/matt_daemon/matt_daemon.log | sed 's/^/   /'
else
    echo "❌ Log file not found"
fi

# Test connection
echo "🔌 Testing connection to port 4242..."
if timeout 5 bash -c "</dev/tcp/localhost/4242"; then
    echo "✅ Port 4242 is accessible"
else
    echo "❌ Cannot connect to port 4242"
fi

# Test sending a message
echo "📤 Sending test message..."
echo "Hello from test script" | timeout 5 telnet localhost 4242 >/dev/null 2>&1 &
sleep 1

# Test quit command
echo "🛑 Sending quit command..."
echo "quit" | timeout 5 telnet localhost 4242 >/dev/null 2>&1

# Wait for daemon to shutdown
sleep 2

# Check if daemon stopped
if kill -0 $DAEMON_PID 2>/dev/null; then
    echo "❌ Daemon still running, forcing shutdown..."
    kill -TERM $DAEMON_PID
    sleep 2
    if kill -0 $DAEMON_PID 2>/dev/null; then
        kill -KILL $DAEMON_PID
    fi
else
    echo "✅ Daemon stopped gracefully"
fi

# Check lock file cleanup
if [ ! -f "/tmp/matt_daemon.lock" ]; then
    echo "✅ Lock file cleaned up"
else
    echo "❌ Lock file still exists"
    rm -f /tmp/matt_daemon.lock
fi

# Show final log entries
if [ -f "/var/log/matt_daemon/matt_daemon.log" ]; then
    echo "📋 Final log entries:"
    tail -10 /var/log/matt_daemon/matt_daemon.log | sed 's/^/   /'
fi

echo "🎉 Test completed!"
