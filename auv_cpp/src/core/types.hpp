#pragma once
#include <array>
#include <string>
#include <variant>
#include <optional>
#include <functional>
#include <chrono>
#include <stdexcept>

// Forward declarations
struct State;
struct SerialState;

/// Log message sources
enum class LogSource {
    MAIN,  // Main system loop
    CTRL,  // Control system
    NAV,   // Navigation
    WSKT,  // WebSocket handler
    LCAL,  // Localization
    PRCP   // Perception
};

/// Log message destinations
enum class LogDest {
    BASE,  // Send to base station
    LOG    // Local logging only
};

/// 3D vector (NED frame)
using Vec3 = std::array<double, 3>;

/// 3x3 matrix
using Mat3x3 = std::array<std::array<double, 3>, 3>;

/**
 * @brief AUV state in NED (North-East-Down) frame
 *
 * Units: meters, m/s, degrees, rad/s
 */
struct State {
    Vec3 position = {0.0, 0.0, 0.0};         ///< [N, E, D] meters
    Vec3 velocity = {0.0, 0.0, 0.0};         ///< [N, E, D] m/s
    Vec3 attitude = {0.0, 0.0, 0.0};         ///< [Roll, Pitch, Yaw] degrees
    Vec3 angular_velocity = {0.0, 0.0, 0.0}; ///< [P, Q, R] rad/s

    State() = default;
    State(const Vec3& pos, const Vec3& vel, const Vec3& att, const Vec3& ang_vel)
        : position(pos), velocity(vel), attitude(att), angular_velocity(ang_vel) {}
};

using SerialState = State;

/// State with body-frame forces and torques
struct ExpandedState : State {
    Vec3 local_force = {0.0, 0.0, 0.0};  ///< [X, Y, Z] Newtons
    Vec3 local_torque = {0.0, 0.0, 0.0}; ///< [X, Y, Z] N·m

    ExpandedState() = default;
    ExpandedState(const State& base, const Vec3& force, const Vec3& torque)
        : State(base), local_force(force), local_torque(torque) {}
};

/// State with mass properties for simulation
struct InitialState : ExpandedState {
    double mass = 0.0;
    Mat3x3 inertia = {{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}};

    InitialState() = default;
};

/**
 * @brief Motor speed commands for 4 thrusters
 *
 * All values in range [-1.0, 1.0]
 */
struct MotorSpeeds {
    double forward = 0.0;
    double turn = 0.0;
    double front = 0.0;
    double back = 0.0;

    MotorSpeeds() = default;

    MotorSpeeds(double fwd, double trn, double fnt, double bck)
        : forward(fwd), turn(trn), front(fnt), back(bck) {
        validate();
    }

    void validate() const {
        for (double speed : {forward, turn, front, back}) {
            if (speed < -1.0 || speed > 1.0) {
                throw std::invalid_argument("speeds must be from -1.0 to 1.0");
            }
        }
    }

    /// Create with values clamped to [-1.0, 1.0]
    static MotorSpeeds clamped(double fwd, double trn, double fnt, double bck) {
        auto clamp = [](double v) { return std::max(-1.0, std::min(1.0, v)); };
        MotorSpeeds result;
        result.forward = clamp(fwd);
        result.turn = clamp(trn);
        result.front = clamp(fnt);
        result.back = clamp(bck);
        return result;
    }
};

using LogContent = std::variant<State, std::string>;

/// Log message for system communication
struct Log {
    LogSource source;
    std::string type;
    LogContent content;
    LogDest dest = LogDest::LOG;

    Log(LogSource src, std::string msg_type, LogContent msg_content,
        LogDest destination = LogDest::LOG)
        : source(src)
        , type(std::move(msg_type))
        , content(std::move(msg_content))
        , dest(destination) {}
};

using Callback = std::function<void()>;

/// Scheduled callback that executes after a duration
struct Promise {
    std::string name;
    double duration;
    Callback callback;
    std::chrono::steady_clock::time_point init_time;

    Promise(std::string promise_name, double duration_seconds, Callback cb)
        : name(std::move(promise_name))
        , duration(duration_seconds)
        , callback(std::move(cb))
        , init_time(std::chrono::steady_clock::now()) {}

    bool is_ready() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - init_time).count();
        return elapsed >= duration;
    }
};

/// Command dispatch entry
struct Dispatch {
    std::string log;
    Callback func;

    Dispatch(std::string log_msg, Callback handler)
        : log(std::move(log_msg)), func(std::move(handler)) {}
};

/// Command from base station
struct Cmd {
    std::string command;
    std::string content;  // JSON string, parsed elsewhere

    Cmd() = default;
    Cmd(std::string cmd, std::string cont)
        : command(std::move(cmd)), content(std::move(cont)) {}
};

constexpr const char* to_string(LogSource source) {
    switch (source) {
        case LogSource::MAIN: return "MAIN";
        case LogSource::CTRL: return "CTRL";
        case LogSource::NAV:  return "NAV";
        case LogSource::WSKT: return "WSKT";
        case LogSource::LCAL: return "LCAL";
        case LogSource::PRCP: return "PRCP";
    }
    return "UNKNOWN";
}

constexpr const char* to_string(LogDest dest) {
    switch (dest) {
        case LogDest::BASE: return "BASE";
        case LogDest::LOG:  return "LOG";
    }
    return "UNKNOWN";
}
