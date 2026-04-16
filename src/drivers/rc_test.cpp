/**
@Class RC Test

Interactive single-motor control via keyboard.
W / Up Arrow   = increase speed (+25us)
S / Down Arrow = decrease speed (-25us)
Q              = quit

Run with sudo for real-time scheduling priority.

Build:
  cd src
  cmake -B build
  cmake --build build
  sudo ./build/rc_test
*/

#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <csignal>
#include "motor.hpp"

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
    #include <termios.h>
    #include <unistd.h>
#endif

// Key codes for arrow keys
constexpr int KEY_UP = 1000;
constexpr int KEY_DOWN = 1001;

#ifndef BUILD_SIMULATION

// Global state for signal handler cleanup
static struct termios g_origTermios;
static bool g_termRawMode = false;
static Motor* g_motor = nullptr;
static int g_gpioHandle = -1;

static void disableRawMode() {
    if (g_termRawMode) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_origTermios);
        g_termRawMode = false;
    }
}

static void enableRawMode() {
    tcgetattr(STDIN_FILENO, &g_origTermios);
    struct termios raw = g_origTermios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    g_termRawMode = true;
}

static void signalHandler(int) {
    std::cout << "\nCaught signal, cleaning up..." << std::endl;
    if (g_motor) {
        g_motor->cleanup();
    }
    if (g_gpioHandle >= 0) {
        lgGpiochipClose(g_gpioHandle);
    }
    disableRawMode();
    _exit(0);
}

// Read a single keypress, detecting arrow key escape sequences
static int readKey() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;

    if (c == 27) { // ESC - might be arrow key
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return 27;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return 27;
        if (seq[0] == '[') {
            if (seq[1] == 'A') return KEY_UP;
            if (seq[1] == 'B') return KEY_DOWN;
        }
        return 27;
    }

    return c;
}

#endif // BUILD_SIMULATION

int main() {
    #ifndef BUILD_SIMULATION
        int handle = lgGpiochipOpen(0);
        if (handle < 0) {
            std::cerr << "failed to initialize lgpio" << std::endl;
            return 1;
        }
        g_gpioHandle = handle;

        int pin = 18; // GPIO 18 = physical pin 12
        Motor motor(pin, handle);
        g_motor = &motor;

        // Install signal handler before arming (Ctrl+C during arm still cleans up)
        signal(SIGINT, signalHandler);

        int result = motor.init();
        if (result < 0) {
            std::cerr << "failed to initialize motor" << std::endl;
            lgGpiochipClose(handle);
            return 1;
        }

        // Arm ESC: hold neutral (1500us) for 7 seconds
        std::cout << "Arming ESC (7 seconds at neutral)..." << std::endl;
        motor.setPwm(1500);
        std::this_thread::sleep_for(std::chrono::seconds(7));
        std::cout << "ESC armed." << std::endl;

        enableRawMode();

        int currentPwm = Motor::NEUTRAL_PWM;

        std::cout << "\n=== RC Motor Control ===" << std::endl;
        std::cout << "W / Up Arrow    = increase PWM (+25us)" << std::endl;
        std::cout << "S / Down Arrow  = decrease PWM (-25us)" << std::endl;
        std::cout << "Q               = quit" << std::endl;
        std::cout << "\nCurrent PWM: " << currentPwm << "us" << std::flush;

        while (true) {
            int key = readKey();

            if (key == 'w' || key == 'W' || key == KEY_UP) {
                currentPwm = std::min(currentPwm + 25, (int)Motor::MAX_PWM);
            } else if (key == 's' || key == 'S' || key == KEY_DOWN) {
                currentPwm = std::max(currentPwm - 25, (int)Motor::MIN_PWM);
            } else if (key == 'q' || key == 'Q') {
                break;
            } else {
                continue;
            }

            motor.setPwm(currentPwm);
            std::cout << "\rCurrent PWM: " << currentPwm << "us   " << std::flush;
        }

        std::cout << std::endl;
        disableRawMode();
        motor.cleanup();
        lgGpiochipClose(handle);
        g_motor = nullptr;
        g_gpioHandle = -1;

    #else
        std::cout << "Simulation mode. no hardware" << std::endl;
    #endif

    std::cout << "RC test complete." << std::endl;
    return 0;
}
