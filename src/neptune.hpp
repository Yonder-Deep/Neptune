#pragma once
#include "drivers/motors/motor.hpp"
#include "drivers/gps/gps.hpp"
#include "states/state.hpp"
#include "drivers/imu/compass.hpp"
#include "types/MotorLocation.hpp"

#include <cstdlib>
#include <unistd.h>
#include <array>
#include <cmath>
#include <iostream>
#include <thread>
#include <numbers>
#include <algorithm>


class Neptune
{
public:
    State *state;
    inline static Neptune *instance = nullptr;
    GPS *gps;
    Motor *front_left;
    Motor *front_right;
    Motor *back_left;
    Motor *back_right;
    Motor *all_motors[4];
    Compass *compass;
    /**
     * Pretty prints all of the sensor data to std out in a nice format
     */
    void dispData()
    {
        // GPS
        try
        {
            auto loc = gps->location();
            std::cout << loc << "\n";
        }
        catch (...)
        {
            std::cout << "GPS Location: <unavailable>\n";
        }

        // Motors
        for (int i = 0; i < 4; ++i)
        {
            Motor *m = all_motors[i];
            if (m)
            {
                double s = 0;
                try
                {
                    s = m->speed;
                }
                catch (...)
                {
                }
                std::cout << "Motor[" << i << "] speed: " << s << "\n";
            }
            else
            {
                std::cout << "Motor[" << i << "]: <null>\n";
            }
        }
        // State
        if (state)
        {
            try
            {
                std::cout << "State tick(ms): " << typeid(state).name() << "\n";
            }
            catch (...)
            {
                std::cout << "State: <unavailable>\n";
            }
        }
        else
        {
            std::cout << "State: <null>\n";
        }
    }
    /**
     * Sets all motors to forward
     *
     * @param max The maximum speed magnitude to apply to each motor. Defaults to 0.75. Bounded [0.0, 1.0] with higher numbers being faster
     */
    void forwards(double max = 0.75)
    {
        front_left->setSpeed(max);
        front_right->setSpeed(max);
        back_left->setSpeed(max);
        back_right->setSpeed(max);
    }
    /**
     * Applies reverse thrust to all motors.
     *
     * @param max The maximum speed magnitude to apply to each motor. Defaults to 0.75. Bounded [0.0, 1.0] with higher numbers being faster
     */
    void backwards(double max = 0.75)
    {
        front_left->setSpeed(-max);
        front_right->setSpeed(-max);
        back_left->setSpeed(-max);
        back_right->setSpeed(-max);
    }

    // Helper to halt all motors. Note this stops the motor, and does not account for drifting
    void stop()
    {
        for (Motor *m : all_motors)
        {
            m->setSpeed(0);
        }
    }

    /**
     * Starts neptune. This method will functionally take over the thread it is called from
     */
    void start()
    {
        Neptune::instance = this;
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

        while (true)
        {
            std::this_thread::sleep_for(this->state->tick());
        }
    }

    void rotate(bool clockwise, double max = 0.75)
    {
        if (clockwise)
        {
            front_left->setSpeed(max);
            front_right->setSpeed(-max);
            back_left->setSpeed(max);
            back_right->setSpeed(-max);
        }
        else
        {
            front_left->setSpeed(-max);
            front_right->setSpeed(max);
            back_left->setSpeed(-max);
            back_right->setSpeed(max);
        }
    }

private:
    // Wraps an angle to [0, 2pi]
    double normalizeToPositiveAngle(double angle)
    {
        while (angle < 0)
        {
            angle += M_PI * 2;
        }
        while (angle > 2 * M_PI)
        {
            angle -= M_PI * 2;
        }
        return angle;
    }
};
