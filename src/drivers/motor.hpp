/**
@Header Motor
@brief Controls a motor via manually generated PWM in a real-time thread.

Uses lgpio only for GPIO pin access (lgGpioWrite), NOT lgTxServo.
A dedicated high-priority thread generates the 50Hz PWM signal
with precise timing to avoid ESC signal loss.

Requires: sudo (for RT scheduling priority)
*/

#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstring>

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
    #include <sched.h>
    #include <sys/mman.h>
    #include <time.h>
    #include <pthread.h>
#endif

class Motor {
    private:
        int pin;
        int handle;
        std::atomic<int> targetPwm;   // pulse width in microseconds
        std::atomic<bool> running;
        std::thread pwmThread;

        #ifndef BUILD_SIMULATION
        // Add nanoseconds to a timespec, handling overflow
        static void addNs(struct timespec& ts, long ns) {
            ts.tv_nsec += ns;
            while (ts.tv_nsec >= 1000000000L) {
                ts.tv_nsec -= 1000000000L;
                ts.tv_sec += 1;
            }
        }

        // The PWM generation loop - runs in a dedicated RT thread
        void pwmLoop() {
            // Pre-fault stack to prevent page faults during RT execution
            constexpr size_t STACK_PREFAULT_SIZE = 16 * 1024;
            volatile char stackPrefault[STACK_PREFAULT_SIZE];
            std::memset(const_cast<char*>(stackPrefault), 0, STACK_PREFAULT_SIZE);

            // Pin to dedicated CPU core (reduces cache migration + IRQ contention)
            cpu_set_t cpuSet;
            CPU_ZERO(&cpuSet);
            CPU_SET(PWM_CPU_CORE, &cpuSet);
            if (pthread_setaffinity_np(pthread_self(), sizeof(cpuSet), &cpuSet) != 0) {
                std::cerr << "warning: could not pin PWM thread to core "
                          << PWM_CPU_CORE << std::endl;
            }

            // Set max real-time scheduling priority
            struct sched_param sp;
            sp.sched_priority = 99;
            if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp) != 0) {
                std::cerr << "warning: could not set RT priority (run with sudo)" << std::endl;
            }

            struct timespec nextPeriod;
            clock_gettime(CLOCK_MONOTONIC, &nextPeriod);

            while (running.load(std::memory_order_relaxed)) {
                int pw = targetPwm.load(std::memory_order_relaxed);

                if (pw > 0) {
                    // Set pin HIGH (start of pulse)
                    lgGpioWrite(handle, pin, 1);

                    // Wait for pulse width duration
                    struct timespec pulseEnd = nextPeriod;
                    addNs(pulseEnd, pw * 1000L); // microseconds to nanoseconds
                    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &pulseEnd, nullptr);

                    // Set pin LOW (end of pulse)
                    lgGpioWrite(handle, pin, 0);
                }

                // Advance to next period (20ms = 50Hz)
                addNs(nextPeriod, 20000000L);
                clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &nextPeriod, nullptr);
            }

            // Ensure pin is LOW when stopping
            lgGpioWrite(handle, pin, 0);
        }
        #endif

    public:
        static constexpr int MIN_PWM = 1100;
        static constexpr int MAX_PWM = 1900;
        static constexpr int NEUTRAL_PWM = 1500;
        static constexpr int PWM_CPU_CORE = 3;  // pin PWM thread here (0-3 on Pi 5)

        Motor(int gpioPin, int gpioHandle)
            : pin(gpioPin)
            , handle(gpioHandle)
            , targetPwm(0)
            , running(false)
        {}

        // Claim the GPIO pin and start the PWM thread
        int init() {
            #ifndef BUILD_SIMULATION
                // Lock memory to prevent page faults in RT thread
                mlockall(MCL_CURRENT | MCL_FUTURE);

                // Claim pin as output
                int result = lgGpioClaimOutput(handle, 0, pin, 0);
                if (result < 0) {
                    std::cerr << "failed to claim GPIO pin " << pin << std::endl;
                    return result;
                }

                // Start PWM generation thread
                running.store(true, std::memory_order_relaxed);
                pwmThread = std::thread(&Motor::pwmLoop, this);
            #else
                std::cout << "simulation: Motor pin " << pin << " initialized" << std::endl;
                running.store(true);
            #endif
            return 0;
        }

        // Set pulse width in microseconds (1100-1900)
        int setPwm(int newPwm) {
            if (newPwm < MIN_PWM || newPwm > MAX_PWM) {
                std::cerr << "pwm value out of range: " << newPwm << std::endl;
                return -1;
            }

            targetPwm.store(newPwm, std::memory_order_relaxed);

            #ifdef BUILD_SIMULATION
                std::cout << "simulation Motor " << pin << " PWM: " << newPwm << std::endl;
            #endif
            return 0;
        }

        void stopMotor() {
            targetPwm.store(0, std::memory_order_relaxed);
            #ifdef BUILD_SIMULATION
                std::cout << "simulation Motor " << pin << " stopped" << std::endl;
            #endif
        }

        int freePin() {
            #ifndef BUILD_SIMULATION
                int result = lgGpioFree(handle, pin);
                return result;
            #else
                return 0;
            #endif
        }

        void cleanup() {
            stopMotor();
            if (running.load()) {
                running.store(false, std::memory_order_relaxed);
                if (pwmThread.joinable()) {
                    pwmThread.join();
                }
            }
            freePin();
        }

        ~Motor() {
            if (running.load()) {
                cleanup();
            }
        }
};
