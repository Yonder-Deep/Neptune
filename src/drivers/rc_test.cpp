/**
@Class RC Test (lgTxPwm version)

Interactive single-motor control via keyboard using lgTxPwm.

W / Up Arrow   = increase speed (+25us)
S / Down Arrow = decrease speed (-25us)
Q              = quit

lgTxPwm(handle, gpio, pwmFrequency, pwmDutyCycle, pwmOffset, pwmCycles)
  - pwmFrequency: Hz (50 for ESC)
  - pwmDutyCycle: 0-100% of period
  - At 50Hz period = 20ms, so duty% = (pulse_us / 20000) * 100
    1100us = 5.5%,  1500us = 7.5%,  1900us = 9.5%

Run with sudo for GPIO access.

Build:
  cd src
  cmake -B build
  cmake --build build
  sudo ./build/rc_test
*/

#include <iostream>
#include <fstream>
#include <algorithm>
#include <csignal>
#include <thread>
#include <chrono>

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
    #include <termios.h>
    #include <unistd.h>
#endif

constexpr int KEY_UP = 1000;
constexpr int KEY_DOWN = 1001;

constexpr int PIN = 18;
constexpr int MIN_PWM = 1100;
constexpr int MAX_PWM = 1900;
constexpr int NEUTRAL_PWM = 1500;
constexpr double PWM_FREQ_HZ = 50.0;  // 50 Hz = 20ms period
constexpr double PERIOD_US = 20000.0;  // 20ms in microseconds

#ifndef BUILD_SIMULATION

static struct termios g_origTermios;
static bool g_termRawMode = false;
static int g_handle = -1;
static std::ofstream g_log;

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
    if (g_log.is_open()) g_log.close();
    if (g_handle >= 0) {
        lgTxPwm(g_handle, PIN, 0, 0, 0, 0);
        lgGpiochipClose(g_handle);
    }
    disableRawMode();
    _exit(0);
}

static auto g_startTime = std::chrono::steady_clock::now();

static void logPwm(const char* event, int pulse_us, double duty, int retval) {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - g_startTime).count();
    g_log << elapsed << "," << event << "," << pulse_us << "," << duty << "," << retval << "\n";
    g_log.flush();
}

static int readKey() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;

    if (c == 27) {
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

// Convert pulse width in microseconds to duty cycle percentage
// e.g. 1500us / 20000us * 100 = 7.5%
static double pwmToDuty(int pulse_us) {
    return (static_cast<double>(pulse_us) / PERIOD_US) * 100.0;
}

#endif // BUILD_SIMULATION

int main() {
    #ifndef BUILD_SIMULATION
        int h = lgGpiochipOpen(0);
        if (h < 0) {
            std::cerr << "failed to initialize lgpio" << std::endl;
            return 1;
        }
        g_handle = h;

        if (lgGpioClaimOutput(h, 0, PIN, 0) != LG_OKAY) {
            std::cerr << "failed to claim GPIO pin " << PIN << std::endl;
            lgGpiochipClose(h);
            return 1;
        }

        signal(SIGINT, signalHandler);

        g_log.open("pwm_log.csv");
        g_log << "time_s,event,pulse_us,duty_pct,retval\n";
        g_startTime = std::chrono::steady_clock::now();

        // Arm ESC: 50Hz, 7.5% duty = 1500us neutral for 7 seconds
        //   lgTxPwm(handle, gpio, freqHz, duty%, offset%, cycles)
        std::cout << "Arming ESC (7 seconds at 1500us neutral)..." << std::endl;
        double armDuty = pwmToDuty(NEUTRAL_PWM);
        int result = lgTxPwm(h, PIN, PWM_FREQ_HZ, armDuty, 0, 0);
        logPwm("arm", NEUTRAL_PWM, armDuty, result);
        if (result < 0) {
            std::cerr << "lgTxPwm failed: " << result << std::endl;
            lgGpiochipClose(h);
            return 1;
        }
        std::this_thread::sleep_for(std::chrono::seconds(7));
        std::cout << "ESC armed." << std::endl;

        enableRawMode();

        int currentPwm = NEUTRAL_PWM;

        std::cout << "\n=== RC Motor Control (lgTxPwm) ===" << std::endl;
        std::cout << "W / Up Arrow    = increase PWM (+25us)" << std::endl;
        std::cout << "S / Down Arrow  = decrease PWM (-25us)" << std::endl;
        std::cout << "Q               = quit" << std::endl;
        std::cout << "\nCurrent PWM: " << currentPwm << "us (duty " << pwmToDuty(currentPwm) << "%)" << std::flush;

        while (true) {
            int key = readKey();

            if (key == 'w' || key == 'W' || key == KEY_UP) {
                currentPwm = std::min(currentPwm + 25, MAX_PWM);
            } else if (key == 's' || key == 'S' || key == KEY_DOWN) {
                currentPwm = std::max(currentPwm - 25, MIN_PWM);
            } else if (key == 'q' || key == 'Q') {
                break;
            } else {
                continue;
            }

            double duty = pwmToDuty(currentPwm);
            int ret = lgTxPwm(h, PIN, PWM_FREQ_HZ, duty, 0, 0);
            logPwm("set", currentPwm, duty, ret);
            std::cout << "\rCurrent PWM: " << currentPwm << "us (duty " << duty << "%)   " << std::flush;
        }

        std::cout << std::endl;
        disableRawMode();
        int stopRet = lgTxPwm(h, PIN, 0, 0, 0, 0);
        logPwm("stop", 0, 0, stopRet);
        if (g_log.is_open()) g_log.close();
        std::cout << "Log saved to pwm_log.csv" << std::endl;
        lgGpiochipClose(h);
        g_handle = -1;

    #else
        std::cout << "Simulation mode. no hardware" << std::endl;
    #endif

    std::cout << "RC test complete." << std::endl;
    return 0;
}
