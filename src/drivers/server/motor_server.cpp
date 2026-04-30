/**
 * @file motor_server.cpp
 * @brief Remote motor control server over TCP.
 *
 * Uses the Thruster class (real hardware) or SimulatedMotor (BUILD_SIMULATION).
 *
 *   Motor API:
 *     - Thruster(int pin, int handle)   -- constructor claims pin; destructor frees it
 *     - motor.armMotor()                -- blocks ~3 s, sets neutral
 *     - motor.setSpeed(float speed)     -- -1.0 (full reverse) .. 0.0 (stop) .. 1.0 (full forward)
 *
 * ## Protocol (newline-terminated JSON)
 *
 *   Set all four motors (normalized -1.0 to 1.0):
 *     {"command":"motors","values":[0.5, 0.0, -0.3, 0.2]}
 *
 *   Set one motor by raw PWM microseconds (1100-1900):
 *     {"command":"pwm","motor":0,"value":1600}
 *     (converted internally to normalized speed before calling setSpeed)
 *
 *   Stop all motors:
 *     {"command":"stop"}
 *
 *   Query current normalized speeds:
 *     {"command":"status"}
 *
 *   Graceful server shutdown:
 *     {"command":"quit"}
 *
 * ## Build & run on Pi
 *   cmake -B build && cmake --build build --target motor_server
 *   sudo ./build/motor_server [port]          (default 9000)
 *
 * ## Simulate (routes motor commands to Gazebo sim over HTTP)
 *   cmake -B build -DBUILD_SIMULATION=ON
 *   ./build/motor_server
 */

#include <csignal>
#include <iostream>
#include <sstream>
#include <string>
#include <array>
#include <memory>
#include <stdexcept>

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
    #include "../motors/thruster.hpp"
#else
    #include "../motors/sim_motor.hpp"
#endif

#include "../../comms/tcp_server.hpp"
// ---------------------------------------------------------------------------
// Constants -- mirrors Thruster internals for the "pwm" command converter
// ---------------------------------------------------------------------------
static constexpr float STOP_SPEED   =  0.0f;
static constexpr int   NEUTRAL_PWM  = 1500;
static constexpr int   MIN_PWM      = 1100;
static constexpr int   MAX_PWM      = 1900;
static constexpr int   MAX_PWM_DIST =  400;   // microseconds from neutral to limit

// ---------------------------------------------------------------------------
// Motor GPIO pin assignments (matches pool_test.cpp / motor_controller.py)
// ---------------------------------------------------------------------------
static constexpr int NUM_MOTORS = 4;
static constexpr int MOTOR_PINS[NUM_MOTORS] = {
    4,   // forward  (FORWARD_GPIO_PIN)
    11,  // turn     (TURN_GPIO_PIN)
    18,  // front    (FRONT_GPIO_PIN)
    24,  // back     (BACK_GPIO_PIN)
};

#ifdef BUILD_SIMULATION
// Map each motor slot to a SimulatedMotor::MotorLocation for the HTTP sim API.
// Mapping is approximate -- adjust once the sim model is finalised.
static constexpr SimulatedMotor::MotorLocation MOTOR_LOCATIONS[NUM_MOTORS] = {
    SimulatedMotor::FrontLeft,   // motor 0 - forward
    SimulatedMotor::FrontRight,  // motor 1 - turn
    SimulatedMotor::BackLeft,    // motor 2 - front
    SimulatedMotor::BackRight,   // motor 3 - back
};
#endif

// Current normalized speed for each motor (source of truth for "status" replies)
static std::array<float, NUM_MOTORS> g_speeds = {
    STOP_SPEED, STOP_SPEED, STOP_SPEED, STOP_SPEED
};

// ---------------------------------------------------------------------------
// Minimal JSON helpers (no external lib)
// ---------------------------------------------------------------------------
static std::string make_response(bool ok, const std::string& msg) {
    std::ostringstream ss;
    ss << "{\"ok\":" << (ok ? "true" : "false")
       << ",\"msg\":\"" << msg << "\"}";
    return ss.str();
}

static std::string make_status(const std::array<float, NUM_MOTORS>& speeds) {
    std::ostringstream ss;
    ss << "{\"ok\":true,\"speeds\":["
       << speeds[0] << "," << speeds[1] << "," << speeds[2] << "," << speeds[3]
       << "]}";
    return ss.str();
}

static std::string parse_string(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos);
    if (pos == std::string::npos) return "";
    size_t end = json.find('"', pos + 1);
    if (end == std::string::npos) return "";
    return json.substr(pos + 1, end - pos - 1);
}

static bool parse_int(const std::string& json, const std::string& key, int& out) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return false;
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    try {
        size_t consumed = 0;
        out = std::stoi(json.substr(pos), &consumed);
        return consumed > 0;
    } catch (...) { return false; }
}

static bool parse_float_array4(const std::string& json, const std::string& key,
                                std::array<float, 4>& out) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return false;
    pos = json.find('[', pos);
    if (pos == std::string::npos) return false;
    pos++;
    for (int i = 0; i < 4; i++) {
        while (pos < json.size() &&
               (json[pos] == ' ' || json[pos] == '\t' || json[pos] == ','))
            pos++;
        if (pos >= json.size() || json[pos] == ']') return false;
        try {
            size_t consumed = 0;
            out[i] = std::stof(json.substr(pos), &consumed);
            pos += consumed;
        } catch (...) { return false; }
    }
    return true;
}

// Convert PWM microseconds (1100-1900) to normalized speed (-1.0 to 1.0).
// Mirrors Thruster::getDutyCycle() in reverse.
static float pwm_to_speed(int pwm) {
    return static_cast<float>(pwm - NEUTRAL_PWM) / static_cast<float>(MAX_PWM_DIST);
}

static void zero_motors(std::array<Motor*, NUM_MOTORS>& motors) {
    for (int i = 0; i < NUM_MOTORS; i++) {
        motors[i]->setSpeed(STOP_SPEED);
        g_speeds[i] = STOP_SPEED;
    }
}

// ---------------------------------------------------------------------------
// Signal handling
// ---------------------------------------------------------------------------
static volatile bool g_running = true;
static void handle_signal(int) { g_running = false; }

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    int port = 9000;
    if (argc >= 2) {
        try { port = std::stoi(argv[1]); }
        catch (...) {
            std::cerr << "Usage: " << argv[0] << " [port]\n";
            return 1;
        }
    }

    std::signal(SIGINT,  handle_signal);
    std::signal(SIGTERM, handle_signal);
    std::signal(SIGPIPE, SIG_IGN);

    // -----------------------------------------------------------------------
    // Open lgpio chip (real hardware only)
    // -----------------------------------------------------------------------
#ifndef BUILD_SIMULATION
    int handle = lgGpiochipOpen(0);
    if (handle < 0) {
        std::cerr << "[INIT] lgGpiochipOpen failed: " << handle << std::endl;
        return 1;
    }
    std::cout << "[INIT] GPIO chip opened." << std::endl;
#else
    std::cout << "[SIM] Simulation mode -- routing to Gazebo sim over HTTP." << std::endl;
#endif

    // -----------------------------------------------------------------------
    // Construct motors
    //
    // Thruster:       constructor claims the GPIO pin; destructor frees it.
    // SimulatedMotor: constructor connects to the HTTP sim server.
    // Both throw on failure, so we catch here and bail cleanly.
    // -----------------------------------------------------------------------
    std::array<std::unique_ptr<Motor>, NUM_MOTORS> motor_storage;

    std::cout << "[INIT] Constructing motors..." << std::endl;
    for (int i = 0; i < NUM_MOTORS; i++) {
        try {
#ifndef BUILD_SIMULATION
            motor_storage[i] = std::make_unique<Thruster>(MOTOR_PINS[i], handle);
#else
            motor_storage[i] = std::make_unique<SimulatedMotor>(MOTOR_LOCATIONS[i]);
#endif
        } catch (const std::exception& e) {
            std::cerr << "[INIT] Failed to construct motor " << i << ": "
                      << e.what() << std::endl;
#ifndef BUILD_SIMULATION
            lgGpiochipClose(handle);
#endif
            return 1;
        }
    }

    // Raw pointer array for convenience in helper functions
    std::array<Motor*, NUM_MOTORS> motors = {
        motor_storage[0].get(), motor_storage[1].get(),
        motor_storage[2].get(), motor_storage[3].get()
    };

    // -----------------------------------------------------------------------
    // Arm ESCs -- armMotor() sets neutral and blocks for the required delay
    // -----------------------------------------------------------------------
    std::cout << "[INIT] Arming ESCs..." << std::endl;
    for (auto* m : motors) m->armMotor();
    std::cout << "[INIT] Motors ready." << std::endl;

    // -----------------------------------------------------------------------
    // TCP server
    // -----------------------------------------------------------------------
    TcpServer server(port);
    try {
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "[TCP] " << e.what() << std::endl;
#ifndef BUILD_SIMULATION
        lgGpiochipClose(handle);
#endif
        return 1;
    }

    // -----------------------------------------------------------------------
    // Main accept loop
    // -----------------------------------------------------------------------
    while (g_running) {
        try {
            server.accept_client();
        } catch (const std::exception& e) {
            std::cerr << "[TCP] accept failed: " << e.what() << std::endl;
            break;
        }

        server.send_line(make_response(true, "connected -- motor_server ready"));

        std::string line;
        while (g_running && server.recv_line(line)) {
            if (line.empty()) continue;
            std::cout << "[CMD] " << line << std::endl;

            std::string cmd = parse_string(line, "command");

            if (cmd == "stop") {
                zero_motors(motors);
                server.send_line(make_response(true, "all motors stopped"));

            } else if (cmd == "motors") {
                std::array<float, 4> vals{};
                if (!parse_float_array4(line, "values", vals)) {
                    server.send_line(make_response(false, "bad values array"));
                    continue;
                }
                bool ok = true;
                for (int i = 0; i < NUM_MOTORS; i++) {
                    // Clamp to [-1, 1] before passing to setSpeed
                    float speed = vals[i];
                    if (speed >  1.0f) speed =  1.0f;
                    if (speed < -1.0f) speed = -1.0f;

                    if (motors[i]->setSpeed(speed) < 0) {
                        server.send_line(make_response(false,
                            "motor " + std::to_string(i) + " setSpeed failed"));
                        ok = false;
                        break;
                    }
                    g_speeds[i] = speed;
                }
                if (ok) server.send_line(make_response(true, "motors updated"));

            } else if (cmd == "pwm") {
                // Accept legacy PWM microseconds (1100-1900) and convert to speed
                int motor_idx = -1, pwm_value = -1;
                if (!parse_int(line, "motor", motor_idx) ||
                    !parse_int(line, "value", pwm_value) ||
                    motor_idx < 0 || motor_idx >= NUM_MOTORS ||
                    pwm_value < MIN_PWM || pwm_value > MAX_PWM) {
                    server.send_line(make_response(false,
                        "bad args -- motor 0-3, value 1100-1900"));
                    continue;
                }
                float speed = pwm_to_speed(pwm_value);
                if (motors[motor_idx]->setSpeed(speed) < 0) {
                    server.send_line(make_response(false, "setSpeed failed"));
                    continue;
                }
                g_speeds[motor_idx] = speed;
                server.send_line(make_response(true,
                    "motor " + std::to_string(motor_idx) +
                    " -> " + std::to_string(pwm_value) + " us"
                    " (speed " + std::to_string(speed) + ")"));

            } else if (cmd == "status") {
                server.send_line(make_status(g_speeds));

            } else if (cmd == "quit") {
                server.send_line(make_response(true, "shutting down"));
                g_running = false;
                break;

            } else {
                server.send_line(make_response(false,
                    "unknown command: " + (cmd.empty() ? "(empty)" : cmd)));
            }
        }

        std::cout << "[SAFE] Client gone -- zeroing motors." << std::endl;
        zero_motors(motors);
    }

    // -----------------------------------------------------------------------
    // Cleanup
    // Thruster destructors free GPIO pins automatically when motor_storage
    // goes out of scope; no manual freePin() calls needed.
    // -----------------------------------------------------------------------
    std::cout << "[SHUTDOWN] Stopping motors." << std::endl;
    for (auto* m : motors) m->setSpeed(STOP_SPEED);

    // motor_storage destructors run here, freeing all pins
#ifndef BUILD_SIMULATION
    lgGpiochipClose(handle);
#endif
    return 0;
}
