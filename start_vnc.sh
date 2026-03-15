#!/bin/bash
# Start virtual framebuffer
Xvfb :1 -screen 0 1920x1080x24 &
sleep 1

# Start a minimal window manager
DISPLAY=:1 fluxbox &
sleep 1

# Start VNC server on display :1, listening on port 5900
x11vnc -display :1 -forever -nopw -rfbport 5900 &

# Start noVNC web proxy (websocket on 6080 → VNC on 5900)
websockify --web /usr/share/novnc 6080 localhost:5900 &

echo "noVNC ready at http://localhost:6080/vnc.html"