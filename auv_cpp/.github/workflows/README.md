# Nautilus AUV - C++ Embedded System

C++ migration of the Nautilus AUV control system for real-time performance.

## Requirements

-   CMake 3.16+
-   GCC 8+ or Clang 10+
-   Eigen 3.3+

### Raspberry Pi

```bash
sudo apt install build-essential cmake libboost-all-dev libeigen3-dev libyaml-cpp-dev pigpio
```

## Build

```bash
cd auv_cpp
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Test

```bash
cd build && ctest --output-on-failure
```

## Project Structure

```
auv_cpp/
├── src/
│   ├── core/        # Types, config, ring buffer
│   ├── drivers/     # Motor, IMU, GPS, depth sensor
│   ├── estimation/  # AHRS, EKF, UKF
│   ├── control/     # PID, thrust allocation
│   └── comms/       # WebSocket server
├── tests/           # Unit and integration tests
└── data/            # Config files
```
