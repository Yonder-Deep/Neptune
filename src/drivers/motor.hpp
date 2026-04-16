/**
 * Motor - 50Hz PWM driver for a BlueRobotics T200 ESC on the Raspberry Pi 5.
 *
 * Spawns one real-time thread per Motor instance that bit-bangs a PWM signal
 * on its GPIO pin via lgpio. The pulse width is held rock-solid (~1us jitter)
 * by doing two things:
 *
 *   1. Running at SCHED_FIFO priority 99 so other userspace can't preempt us.
 *   2. Busy-waiting for the pulse duration (instead of clock_nanosleep),
 *      anchored to the ACTUAL HIGH-write timestamp instead of a planned time.
 *      This cancels out the kernel's ~10-50us sleep wakeup jitter, which
 *      would otherwise wobble the pulse width and make the ESC lose sync.
 *
 * The 20ms period boundary between pulses IS still done with clock_nanosleep,
 * because wakeup jitter there only shifts when the next pulse starts — it
 * doesn't affect the pulse width itself, and the ESC doesn't care about the
 * period to the microsecond.
 *
 * Requires sudo (SCHED_FIFO priority + /dev/gpiochip0 access).
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <iostream>
#include <thread>

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
    #include <pthread.h>
    #include <sched.h>
    #include <sys/mman.h>
    #include <time.h>
#endif

class Motor {
public:
    static constexpr int MIN_PWM = 1100;
    static constexpr int MAX_PWM = 1900;
    static constexpr int NEUTRAL_PWM = 1500;

    Motor(int gpioPin, int gpioHandle)
        : pin(gpioPin), handle(gpioHandle) {}

    ~Motor() { cleanup(); }

    // Claim the GPIO pin and start the PWM thread.
    int init() {
#ifndef BUILD_SIMULATION
        // Keep all pages resident so the RT thread never page-faults.
        mlockall(MCL_CURRENT | MCL_FUTURE);

        int result = lgGpioClaimOutput(handle, 0, pin, 0);
        if (result < 0) {
            std::cerr << "failed to claim GPIO pin " << pin << "\n";
            return result;
        }

        running.store(true);
        pwmThread = std::thread(&Motor::pwmLoop, this);
#else
        std::cout << "[sim] motor on pin " << pin << " initialized\n";
#endif
        return 0;
    }

    // Set pulse width in microseconds (1100-1900). 0 = no pulses (motor off).
    int setPwm(int pwm) {
        if (pwm != 0 && (pwm < MIN_PWM || pwm > MAX_PWM)) {
            std::cerr << "pwm out of range: " << pwm << "\n";
            return -1;
        }
        targetPwm.store(pwm);
#ifdef BUILD_SIMULATION
        std::cout << "[sim] pin " << pin << " pwm = " << pwm << "\n";
#endif
        return 0;
    }

    void stopMotor() { targetPwm.store(0); }

    // Stop the PWM thread and release the pin. Safe to call multiple times.
    void cleanup() {
        // Atomic swap: only the first caller to see running=true does the work.
        if (!running.exchange(false)) return;
        if (pwmThread.joinable()) pwmThread.join();
#ifndef BUILD_SIMULATION
        lgGpioFree(handle, pin);
#endif
    }

private:
    int pin;
    int handle;
    std::atomic<int>  targetPwm{0};   // pulse width in microseconds
    std::atomic<bool> running{false};
    std::thread       pwmThread;

    static constexpr int64_t PERIOD_NS = 20'000'000;  // 20ms = 50Hz

#ifndef BUILD_SIMULATION
    // Current monotonic time in nanoseconds. Uses vDSO, so no syscall (~20ns).
    static int64_t nowNs() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return int64_t(ts.tv_sec) * 1'000'000'000 + ts.tv_nsec;
    }

    // Sleep until absolute monotonic time. Wakeup has ~10-50us of jitter.
    static void sleepUntilNs(int64_t targetNs) {
        struct timespec ts;
        ts.tv_sec  = targetNs / 1'000'000'000;
        ts.tv_nsec = targetNs % 1'000'000'000;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, nullptr);
    }

    // The PWM generation loop. Runs in its own RT thread.
    void pwmLoop() {
        // Max real-time priority so nothing else in userspace can preempt us.
        struct sched_param sp;
        sp.sched_priority = 99;
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp) != 0) {
            std::cerr << "warning: could not set RT priority (run with sudo)\n";
        }

        int64_t nextPeriod = nowNs();

        while (running.load()) {
            // Wait for the start of the next 20ms period. Wakeup jitter
            // here is fine — it doesn't affect the pulse width below.
            sleepUntilNs(nextPeriod);

            int pw = targetPwm.load();
            if (pw > 0) {
                // 1. Raise the pin.
                // 2. Immediately record the real HIGH time.
                // 3. Busy-wait until (realHighTime + pw microseconds).
                // 4. Drop the pin.
                //
                // Anchoring to the REAL HIGH time (not the planned period
                // start) means any jitter in step 1 is baked into both the
                // start and the end of the pulse — so it cancels out of the
                // pulse width. That's the trick that fixes ESC chatter.
                lgGpioWrite(handle, pin, 1);
                int64_t pulseEnd = nowNs() + int64_t(pw) * 1000;
                while (nowNs() < pulseEnd) { /* spin, ~20ns per iter */ }
                lgGpioWrite(handle, pin, 0);
            }

            nextPeriod += PERIOD_NS;
        }

        lgGpioWrite(handle, pin, 0);  // failsafe: pin LOW on thread exit
    }
#endif
};
