# Neptune Base Station - Log Client

A simple WebSocket client that connects to the Neptune AUV server and receives real-time log messages.

## Features

- Connects to WebSocket server at `127.0.0.1:8080`
- Receives and displays all log messages in real-time
- Graceful shutdown with Ctrl+C
- Thread-safe async I/O using websocketpp

## Building

### Prerequisites

- C++17 compiler (g++ 7+)
- CMake 3.16+
- websocketpp headers (typically in `/usr/include/websocketpp`)
- Boost ASIO

### Build Steps

```bash
cd /workspaces/Neptune/base
mkdir -p build
cd build
cmake ..
make
```

The executable will be in `build/bin/log_client`

## Running

### Start the AUV server (in separate terminal)

```bash
cd /workspaces/Neptune/auv_cpp/build/bin
./auv_control /workspaces/Neptune/auv_cpp/data/config.yaml
```

Make sure `config.yaml` is configured to listen on `127.0.0.1:8080`:

```yaml
network:
  ip: "127.0.0.1"
  port: 8080
```

### Start the log client

```bash
cd /workspaces/Neptune/base/build/bin
./log_client
```

## Example Output

```
[CLIENT] Connecting to ws://127.0.0.1:8080...
[CLIENT] Connected to server
[LOG] [MAIN] Logger initialized
[LOG] [MAIN] System mode - Simulation: enabled, Perception: disabled
[LOG] [MAIN] Network configured - IP: 127.0.0.1, Port: 8080
[LOG] [MAIN] WebSocket server is now running
[LOG] [MAIN] Log broadcast to WebSocket clients enabled
[LOG] [MAIN] Entering main control loop
[LOG] [MAIN] Vehicle state initialized at origin
...
```

## Architecture

The client:
1. Connects to the WebSocket server running on the AUV
2. Registers callbacks for connection/message/error events
3. Runs the ASIO event loop in a separate thread
4. Receives log messages and prints them to stdout
5. Can be stopped gracefully with Ctrl+C

## Notes

- Logs are broadcast from the server every time a log call is made
- Messages are sent in real-time (no buffering)
- The client will automatically reconnect if the server restarts (modify `on_fail` for retry logic)
- Timestamps are included in each log message from the server

## Future Enhancements

- File logging of received messages
- Log filtering by source (MAIN, WSKT, NAV, etc.)
- Multiple client connections
- Live log parsing and analysis
- GUI for log visualization
# Neptune Base Station

Ground control software for the Neptune AUV. This directory contains the base station applications that communicate with the embedded control system via WebSocket.

## Log Receiver

A real-time log receiver that connects to the embedded WebSocket server and displays/records all system logs from the AUV.

### Setup

Install dependencies:
```bash
pip install -r requirements.txt
```

### Usage

**Basic (local development):**
```bash
python3 log_receiver.py
```

**Connect to specific server:**
```bash
python3 log_receiver.py --host 192.168.1.100 --port 8080
```

**Custom log file location:**
```bash
python3 log_receiver.py --logfile /var/log/neptune_base.log
```

**View help:**
```bash
python3 log_receiver.py --help
```

### Example Output

```
Connecting to ws://127.0.0.1:8080...
Connected to 127.0.0.1:8080
Logging to: /workspaces/Neptune/base/logs/base_station.log
Receiving logs (Ctrl+C to exit)...

[2026-04-09 14:30:45.123] [MAIN] Logger initialized
[2026-04-09 14:30:45.124] [MAIN] System mode - Simulation: enabled, Perception: disabled
[2026-04-09 14:30:45.125] [MAIN] Network configured - IP: 127.0.0.1, Port: 8080
[2026-04-09 14:30:45.126] [MAIN] Initializing WebSocket server on 127.0.0.1:8080
[2026-04-09 14:30:45.127] [WSKT] listening on 127.0.0.1:8080
[2026-04-09 14:30:45.128] [MAIN] WebSocket server is now running
[2026-04-09 14:30:45.129] [MAIN] Log broadcast to WebSocket clients enabled
[2026-04-09 14:30:45.130] [MAIN] Entering main control loop
```

### Features

- **Real-time streaming** - Logs appear in console as they're generated
- **File persistence** - All logs saved to `logs/base_station.log` (timestamped)
- **Automatic reconnection handling** - Gracefully handles disconnections
- **Ping/pong monitoring** - Detects lost connections
- **Command-line flexibility** - Configure host, port, and log file location

### Development Notes

The log receiver is a simple WebSocket client that:
1. Connects to the embedded AUV's WebSocket server
2. Receives log messages in real-time via the broadcast callback
3. Displays them with timestamps on the console
4. Saves them to a file for historical reference

For the simulation environment at `127.0.0.1:8080`:
```bash
python3 log_receiver.py --host 127.0.0.1 --port 8080
```

## Future Extensions

Additional base station applications could include:
- Real-time telemetry dashboard (position, attitude, sensor readings)
- Mission planner and executor interface
- Manual waypoint input during testing
- Data visualization and playback
- Live video stream receiver (when depth camera integration is added)
