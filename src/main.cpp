#include <iostream>
#include <string>
#include <csignal>

#include <lgpio.h>

#include "winch.hpp"
#include "stepper_motor.hpp"

static constexpr int STEP_PIN   = 18; 
static constexpr int DIR_PIN    = 23;  
static constexpr int ENABLE_PIN = 24; 
static volatile std::sig_atomic_t g_should_exit = 0;

void on_sigint(int) {
    g_should_exit = 1;
}

static void print_help() {
    std::cout <<
        "Commands:\n"
        "  f <steps>   : Reel In  (raise hydrophone)\n"
        "  g <steps>   : reel OUT (lower hydrophone)\n"
        "  s           : STOP (immediate)\n"
        "  v <sps>     : set speed (steps/sec)\n"
        "  h           : help\n"
        "  q           : quit\n"
        "Notes:\n"
        " - Your current Stepper::move_steps() is blocking while it steps.\n"
        " - STOP works by setting an atomic flag checked each step.\n";
}

int main() {
    std::signal(SIGINT, on_sigint);

    if (gpioInitialise() < 0) {
        std::cerr << "pigpio init failed\n";
        return 1;
    }

    Stepper stepper(STEP_PIN, DIR_PIN, ENABLE_PIN, /*enableActiveLow=*/true);
    Winch winch(stepper);

    winch.init();
    winch.set_speed(500.0f);  //default

    print_help();

    std::string line;
    while (!g_should_exit) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        char cmd = 0;
        long value = 0;

        cmd = line[0];

        if (cmd == 'q' || cmd == 'Q') break;
        if (cmd == 'h' || cmd == 'H' || cmd == '?') { print_help(); continue; }

        if (cmd == 's' || cmd == 'S') {
            std::cout << "STOP\n";
            winch.stop();
            continue;
        }

        try {
            if (line.size() > 1) {
                size_t pos = line.find_first_of("-0123456789");
                if (pos != std::string::npos) value = std::stol(line.substr(pos));
            }
        } catch (...) {
            std::cout << "Could not parse number\n";
            continue;
        }

        switch (cmd) {
            case 'v':
            case 'V':
                if (value <= 0) {
                    std::cout << "Speed must be > 0\n";
                } else {
                    std::cout << "Set speed to " << value << " steps/sec\n";
                    winch.set_speed((float)value);
                }
                break;

            case 'f':
            case 'F':
                if (value <= 0) value = 200; 
                std::cout << "Reel IN " << value << " steps\n";
                winch.reel_in_steps((int32_t)value);
                break;

            case 'g':
            case 'G':
                if (value <= 0) value = 200;
                std::cout << "Reel OUT " << value << " steps\n";
                winch.reel_out_steps((int32_t)value);
                break;

            default:
                std::cout << "Unknown command. Type 'h' for help.\n";
                break;
        }
    }

    std::cout << "\nExiting. Disabling motor.\n";
    stepper.disable();
    gpioTerminate();
    return 0;
}
