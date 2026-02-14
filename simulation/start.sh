#!/bin/bash
set -e

echo "Starting PyBullet simulation with VNC streaming..."

# 1. Start virtual display in the background
echo "Starting Xvfb virtual display..."
Xvfb :1 -screen 0 1024x768x24 &
XVFB_PID=$!
sleep 2  # Wait for Xvfb to be ready

# 2. Start lightweight window manager (fluxbox)
echo "Starting window manager..."
DISPLAY=:1 fluxbox &
sleep 1

# 3. Start VNC server (mapping the Xvfb display)
echo "Starting x11vnc server..."
x11vnc -display :1 -nopw -forever -quiet -bg

# 4. Start websockify bridge (converts VNC to WebSockets for browser)
# Port 6080 is the default
echo "Starting websockify bridge..."

# Find noVNC web directory (try common locations)
NOVNC_WEB=""
for path in /usr/share/novnc /usr/local/share/novnc /opt/novnc; do
    if [ -d "$path" ]; then
        NOVNC_WEB="$path"
        break
    fi
done

if [ -z "$NOVNC_WEB" ]; then
    echo "Warning: noVNC web directory not found. Starting websockify without web interface."
    websockify 6080 localhost:5900 &
else
    echo "Using noVNC from: $NOVNC_WEB"
    websockify --web=$NOVNC_WEB 6080 localhost:5900 &
fi

WEBSOCKIFY_PID=$!
sleep 1

# 5. Run your PyBullet script (ensure it uses p.connect(p.GUI))
echo "Starting PyBullet simulation..."
export DISPLAY=:1
python main.py

# Cleanup on exit
trap "kill $XVFB_PID $WEBSOCKIFY_PID 2>/dev/null || true" EXIT