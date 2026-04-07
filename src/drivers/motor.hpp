/**
@Header Motor
@brief Controls a motor via hardware PWM using the Linux sysfs interface.

Requires dtoverlay=pwm-2chan in /boot/firmware/config.txt
PWM channel 0 = GPIO 18, PWM channel 1 = GPIO 19
*/

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

class Motor {
    private:
        int channel;        // PWM channel (0 for GPIO 18, 1 for GPIO 19)
        int pwm;            // current pulse width in microseconds
        std::string base;   // sysfs path for this PWM channel
        bool exported;

        bool sysfsWrite(const std::string& file, const std::string& value) {
            std::ofstream ofs(base + "/" + file);
            if (!ofs.is_open()) {
                std::cerr << "failed to open " << base << "/" << file << std::endl;
                return false;
            }
            ofs << value;
            ofs.close();
            return true;
        }

    public:
        static constexpr int MIN_PWM = 1100;
        static constexpr int MAX_PWM = 1900;
        static constexpr int NEUTRAL_PWM = 1500;
        static constexpr int PERIOD_NS = 20000000; // 20ms = 50Hz

        // channel: 0 for GPIO 18, 1 for GPIO 19
        Motor(int pwmChannel)
            : channel(pwmChannel)
            , pwm(NEUTRAL_PWM)
            , base("/sys/class/pwm/pwmchip0/pwm" + std::to_string(pwmChannel))
            , exported(false)
        {}

        // Export the PWM channel and configure 50Hz period
        int init() {
            #ifndef BUILD_SIMULATION
                // Export the channel
                std::ofstream exportFs("/sys/class/pwm/pwmchip0/export");
                if (!exportFs.is_open()) {
                    std::cerr << "failed to open pwmchip0/export (already exported?)" << std::endl;
                    // May already be exported, try to continue
                }  else {
                    exportFs << channel;
                    exportFs.close();
                    // Brief delay for sysfs to create the directory
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                // Set period (50Hz = 20ms)
                if (!sysfsWrite("period", std::to_string(PERIOD_NS))) {
                    std::cerr << "failed to set period" << std::endl;
                    return -1;
                }

                // Set initial duty cycle to neutral
                if (!sysfsWrite("duty_cycle", std::to_string(NEUTRAL_PWM * 1000))) {
                    std::cerr << "failed to set initial duty_cycle" << std::endl;
                    return -1;
                }

                // Enable
                if (!sysfsWrite("enable", "1")) {
                    std::cerr << "failed to enable PWM" << std::endl;
                    return -1;
                }

                exported = true;
            #else
                std::cout << "simulation: Motor channel " << channel << " initialized" << std::endl;
                exported = true;
            #endif
            return 0;
        }

        // Set pulse width in microseconds (1100-1900)
        int setPwm(int newPwm) {
            if (newPwm < MIN_PWM || newPwm > MAX_PWM) {
                std::cerr << "pwm value out of range: " << newPwm << std::endl;
                return -1;
            }

            pwm = newPwm;

            #ifndef BUILD_SIMULATION
                // Convert microseconds to nanoseconds for sysfs
                if (!sysfsWrite("duty_cycle", std::to_string(pwm * 1000))) {
                    std::cerr << "failed to set pwm" << std::endl;
                    return -1;
                }
            #else
                std::cout << "simulation Motor ch" << channel << " PWM: " << pwm << std::endl;
            #endif
            return 0;
        }

        void stopMotor() {
            #ifndef BUILD_SIMULATION
                sysfsWrite("duty_cycle", "0");
                sysfsWrite("enable", "0");
            #else
                std::cout << "simulation Motor ch" << channel << " stopped" << std::endl;
            #endif
        }

        void cleanup() {
            if (exported) {
                stopMotor();
                // Unexport the channel
                std::ofstream unexportFs("/sys/class/pwm/pwmchip0/unexport");
                if (unexportFs.is_open()) {
                    unexportFs << channel;
                    unexportFs.close();
                }
                exported = false;
            }
        }

        ~Motor() {
            cleanup();
        }
};
