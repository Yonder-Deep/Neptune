/**
@Class manual_test
@brief Drives the 4-motor ThrusterController from stdin.

Reads three floats per line: forward strafe yaw  (each in [-1, 1]).
For example:
    > 1.0 0.0 0.0     # full forward
    > 0.0 0.0 1.0     # full yaw CCW
    > 0.5 0.5 0.0     # forward + strafe right
    > 0 0 0           # all stop
    > q               # quit

In simulation mode this POSTs to http://localhost:6767/motor/<name>/set,
so Webots must be running with the Neptune world loaded and ▶ pressed.

Build (sim):
    cmake -B build -DBUILD_SIMULATION=ON
    cmake --build build
    ./build/manual_test
*/

#include <iostream>
#include <string>
#include <sstream>
#include "motor.hpp"
#include "thruster_controller.hpp"

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
#endif

int main() {
    #ifdef BUILD_SIMULATION
        // Sim mode: pins/handles unused, names map to Webots Flask endpoints.
        Motor lt(0, 0, "top_left");
        Motor rt(0, 0, "top_right");
        Motor lb(0, 0, "bottom_left");
        Motor rb(0, 0, "bottom_right");
    #else
        int handle = lgGpiochipOpen(0);
        if (handle < 0) { std::cerr << "lgpio open failed\n"; return 1; }
        // GPIO pins from pool_test.cpp: front=24 right=11 left=4 back=18.
        // Re-assign here once your real wiring is locked in.
        Motor lt( 4, handle);
        Motor rt(11, handle);
        Motor lb(24, handle);
        Motor rb(18, handle);
    #endif

    if (lt.init() < 0 || rt.init() < 0 || lb.init() < 0 || rb.init() < 0) {
        std::cerr << "motor init failed\n";
        return 1;
    }

    // Arm: hold neutral for 7 seconds (real ESCs need this; sim is harmless).
    std::cout << "Arming (7 s neutral)..." << std::endl;
    lt.setPwm(Motor::NEUTRAL_PWM); rt.setPwm(Motor::NEUTRAL_PWM);
    lb.setPwm(Motor::NEUTRAL_PWM); rb.setPwm(Motor::NEUTRAL_PWM);
    std::this_thread::sleep_for(std::chrono::seconds(7));
    std::cout << "Armed. Enter: forward strafe yaw  (each in [-1,1]). 'q' to quit." << std::endl;

    ThrusterController controller(lt, rt, lb, rb);

    std::string line;
    while (std::cout << "> " && std::getline(std::cin, line)) {
        if (line.empty()) continue;
        if (line[0] == 'q' || line[0] == 'Q') break;
        std::istringstream iss(line);
        float f, s, y;
        if (!(iss >> f >> s >> y)) {
            std::cerr << "  parse error; need three floats\n";
            continue;
        }
        controller.setAxes(f, s, y);
        std::cout << "  -> sent forward=" << f << " strafe=" << s << " yaw=" << y << "\n";
    }

    std::cout << "Stopping motors..." << std::endl;
    controller.setAxes(0, 0, 0);
    lt.cleanup(); rt.cleanup(); lb.cleanup(); rb.cleanup();

    #ifndef BUILD_SIMULATION
        lgGpiochipClose(handle);
    #endif

    return 0;
}
