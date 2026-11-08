#pragma once

#include "drivers/gps/gps.hpp"
#include "drivers/imu/compass.hpp"
#include "drivers/motors/motor.hpp"
#include "states/state.hpp"

#include "rfl/Literal.hpp"
#include "rfl/TaggedUnion.hpp"
#include "rfl/visit.hpp"
#include "rfl/yaml/write.hpp"
#include <concepts>
#include <rfl.hpp>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <variant>
#include <vector>

// ====================================================
// These are some fake example network message types
// ====================================================

struct InfoLog {
    using Tag = rfl::Literal<"info">;

    std::string message;
};

struct ErrorLog {
    using Tag = rfl::Literal<"err">;

    int32_t code;
    std::string message;
    std::vector<std::string> stack_trace;
};

struct MetricLog {
    using Tag = rfl::Literal<"metric">;

    std::string metric_name;
    double value;
};

// ====================================================
// Putting them into a serializable tagged union & allowing transfer
// to a simple, plain std::variant
// ====================================================

using RflMessage = rfl::TaggedUnion<"name", InfoLog, ErrorLog, MetricLog>;

using Message = std::variant<InfoLog, ErrorLog, MetricLog>;

inline Message to_std_variant(const RflMessage &message) {
    return rfl::visit([](const auto &value) -> Message { return Message{value}; }, message);
}

// DELETE THIS
inline void type_testing() {
    {
        const RflMessage foo = InfoLog("Hello");
        auto *bar = new std::ofstream();
        auto &serialized = rfl::yaml::write(foo, *bar);
    }
    {
        const RflMessage foo = InfoLog("Hello");
        auto bar = to_std_variant(foo);
    }
}

// ====================================================
// Multiple "modes" that the robot can be in. For each mode,
// it has a different set of enabled behaviors & handlers,
// as well as whatever arbitrary state is desired.
// ====================================================

// Lambda overloads
template <class... Ts> struct Overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

class Mode1 {
  public:
    int some_state;

    using AcceptedMessages = std::variant<InfoLog, ErrorLog>;

    template <typename ModeTransitionFn>
    auto handlers(ModeTransitionFn &transition_mode);
};

class Mode2 {
  public:
    int some_other_state;

    Mode2(int some_state) : some_other_state(some_state) {};

    using AcceptedMessages = std::variant<MetricLog>;

    template <typename ModeTransitionFn>
    auto handlers(ModeTransitionFn &transition_mode);
};

// All possible modes
using Mode = std::variant<Mode1, Mode2>;

// Handlers need to be defined after all modes so transitions can be fully qualified
// TODO: Figure out a way around that to consolidate everything inside class declaration

template <typename ModeTransitionFn>
inline auto Mode1::handlers(ModeTransitionFn &transition_to_mode) {
    return Overloaded{
        [this](const InfoLog &m) {
            std::cout << "Info log";
            some_state++;
        },
        [this, &transition_to_mode](const ErrorLog &m) {
            std::cout << "Some error, switching modes";
            if (some_state > 10) {
                transition_to_mode(Mode2(some_state));
            }
        },
    };
}

template <typename ModeTransitionFn>
inline auto Mode2::handlers(ModeTransitionFn &transition_to_mode) {
    return Overloaded{
        [this](const MetricLog &m) {
            std::cout << "Some collected state :" << some_other_state;
            std::cout << "Metric name: " << m.metric_name;
            std::cout << "Metric value: " << m.value;
            some_other_state--;
        },
    };
};

/// Returns true when `T` is an alternative of the input variants.
/// Being used to enforce that the active 
template<typename T, typename Variant>
struct IsAlternative : std::false_type {};
template<typename T, typename... Alts>
struct IsAlternative<T, std::variant<Alts...>>
    : std::bool_constant<(std::same_as<std::remove_cvref_t<T>, Alts> || ...)> {};

class Neptune {
  public:
    State *state;
    GPS *gps;
    Motor *front_left;
    Motor *front_right;
    Motor *back_left;
    Motor *back_right;
    Motor *all_motors[4];
    Compass *compass;

    Mode mode;

    void dispatch(const Message &msg) {
        // Closure to possibly transition to the new mode
        // Safety: Do not access the current mode after calling this, calling
        //         this should be the last statement in each handler
        auto transition_to_mode= [this](/*ValidModeFor<Mode>*/ auto next_state) {
            mode = std::move(next_state);
        };

        std::visit([&]<typename ActiveMode, typename ActiveMsg>(ActiveMode& active_mode, const ActiveMsg& active_msg) {
            using AcceptedMessages = ActiveMode::AcceptedMessages;

            if constexpr (IsAlternative<ActiveMsg, AcceptedMessages>::value) {
                const ActiveMsg& refined_msg = active_msg;
                active_mode.handlers(transition_to_mode)(refined_msg);
            } else {
                // Eventually handle the error here in some way
                std::cout << "Message ignored in current mode.\n";
            }
        }, mode, msg);
    }

    /**
     * Pretty prints all of the sensor data to std out in a nice format
     */
    void dispData() {
        // GPS
        try {
            auto loc = gps->location();
            std::cout << loc << "\n";
        } catch (...) {
            std::cout << "GPS Location: <unavailable>\n";
        }

        // Motors
        for (int i = 0; i < 4; ++i) {
            Motor *m = all_motors[i];
            if (m) {
                double s = 0;
                try {
                    s = m->speed;
                } catch (...) {
                }
                std::cout << "Motor[" << i << "] speed: " << s << "\n";
            } else {
                std::cout << "Motor[" << i << "]: <null>\n";
            }
        }
        // State
        if (state) {
            try {
                std::cout << "State tick(ms): " << typeid(state).name() << "\n";
            } catch (...) {
                std::cout << "State: <unavailable>\n";
            }
        } else {
            std::cout << "State: <null>\n";
        }
    }
    /**
     * Sets all motors to forward
     *
     * @param max The maximum speed magnitude to apply to each motor. Defaults to 0.75. Bounded
     * [0.0, 1.0] with higher numbers being faster
     */
    void forwards(double max = 0.75) {
        front_left->setSpeed(max);
        front_right->setSpeed(max);
        back_left->setSpeed(max);
        back_right->setSpeed(max);
    }
    /**
     * Applies reverse thrust to all motors.
     *
     * @param max The maximum speed magnitude to apply to each motor. Defaults to 0.75. Bounded
     * [0.0, 1.0] with higher numbers being faster
     */
    void backwards(double max = 0.75) {
        front_left->setSpeed(-max);
        front_right->setSpeed(-max);
        back_left->setSpeed(-max);
        back_right->setSpeed(-max);
    }

    // Helper to halt all motors. Note this stops the motor, and does not account for drifting
    void stop() {
        for (Motor *m : all_motors) {
            m->setSpeed(0);
        }
    }

    /**
     * Starts neptune. This method will functionally take over the thread it is called from
     */
    void start() {
        // gps->await_lock(5, 500);
        std::cout << "Arming motors" << std::endl;
        front_left->armMotor();
        front_right->armMotor();
        back_left->armMotor();
        back_right->armMotor();

        all_motors[0] = front_left;
        all_motors[1] = front_right;
        all_motors[2] = back_left;
        all_motors[3] = back_right;

        while (true) {
            std::this_thread::sleep_for(this->state->tick());
        }
    }

    void rotate(bool clockwise, double max = 0.75) {
        if (clockwise) {
            front_left->setSpeed(max);
            front_right->setSpeed(-max);
            back_left->setSpeed(max);
            back_right->setSpeed(-max);
        } else {
            front_left->setSpeed(-max);
            front_right->setSpeed(max);
            back_left->setSpeed(-max);
            back_right->setSpeed(max);
        }
    }

  private:
    // Wraps an angle to [0, 2pi]
    double normalizeToPositiveAngle(double angle) {
        while (angle < 0) {
            angle += M_PI * 2;
        }
        while (angle > 2 * M_PI) {
            angle -= M_PI * 2;
        }
        return angle;
    }
};
