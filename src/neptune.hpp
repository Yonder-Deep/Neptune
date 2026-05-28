#pragma once
#include "drivers/motors/motor.hpp"
#include "drivers/gps/gps.hpp"

#include "drivers/imu/compass.hpp"
#include <cstdlib>
#include <unistd.h>
#include <array>
#include <cmath>
#include <iostream>
#include <thread>
#include <numbers>
#include "types/MotorLocation.hpp"
#include <cmath>
#include <algorithm>

#include "states/state.hpp"

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
     * Prints all of the sensor data to std out in a nice format
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
        // Compass
        try
        {
            auto c = compass->get();
            std::cout << "Compass: [" << c[0] << ", " << c[1] << ", " << c[2] << "] Heading(rad): " << get_heading() << "\n";
        }
        catch (...)
        {
            std::cout << "Compass: <unavailable>\n";
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
    // Helper to power motors for forward movement
    void forwards(double max = 0.75)
    {
        front_left->setSpeed(max);
        front_right->setSpeed(max);
        back_left->setSpeed(max);
        back_right->setSpeed(max);
    }
    // Helper to power motors for backwards movement
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
     * Gets the heading, in radians of the robot. 0 indicates north, pi indicates south
     * Heading is a **counterclockwise** rotation about the upward axsis
     * @return The heading
     */
    double get_heading()
    {
        auto c = compass->get(); // north in the format: [nx, ny, nz] of the compass
        // We know true north in ENU is [0,0,1], so we need to measure our north directions difference from that
        // Ignore z(upwards) cause if we get rotated too far upward it impacts nav we're fucked anyway. Thus our axis of rotation is +z (0,0,1)

        // Flip x/y here because atan will give from x axsis but we want from +y
        double out = atan2(c[1], c[0]);

        return out;
    }
    const double MAX_MOTOR_ACTIVATION = 0.75; // TODO move to some config or smth
    /**
     * Activates the motors to travel towards the target location.
     * Uses MAX_MOTOR_ACTIVATION to scale the output
     * TODO change tolerance, 5.0 is crazy high for gps coords
     * @return if the location is reached
     */
    bool travel(GPSCoordinate goal, double dist_tolerance = 1.0, double rot_tolerance = (0.174533 / 2)) // 5 deg
    {
        Vector2d dif = goal - gps->location();
        if (dif.norm() < dist_tolerance)
        {
            stop();
            return true;
        }
        // Find a way to move along dif with our motors

        double heading = get_heading();
        double goal_heading = atan2(dif[1], dif[0]);
        double angle_error = atan2(
            sin(goal_heading - heading),
            cos(goal_heading - heading));

        std::cout << "Heading: " << heading << "goal" << goal_heading << "dif: " << angle_error << std::endl;
        if (angle_error < rot_tolerance)
        {
            // move forward
            forwards();
        }
        else
        {
            // rotate self
            rotate((angle_error < 0), 0.3);
        }
        return false;
    }
    /**
     * Starts neptune. This method will functionally take over the thread it is called from
     */
    void start()
    {
        Neptune::instance = this;
        gps->await_lock(5, 500);
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
    // Rotates
    void rotate(bool clockwise, double max = 0.75)
    {
        if (clockwise)
        {
            // rights on backwards, lefts on forward
            front_left->setSpeed(max);
            front_right->setSpeed(-max);
            back_left->setSpeed(max);
            back_right->setSpeed(-max);
        }
        else
        {
            // inverse
            front_left->setSpeed(-max);
            front_right->setSpeed(max);
            back_left->setSpeed(-max);
            back_right->setSpeed(max);
        }
    }

private:
    // Helper to build a matrix of the motors thrust vectors, rotate by the heading
    Matrix<double, 2, 4> motor_mat()
    {
        Matrix<double, 2, 4> out;
        std::cout << "heading" << get_heading() << std::endl;
        Eigen::Rotation2D<double> rot(get_heading());
        for (int m = static_cast<int>(MotorLocation::FrontRight); m <= static_cast<int>(MotorLocation::BackLeft); m++)
        {
            out.col(m) = rot * thrust_vector(static_cast<MotorLocation>(m)); // rotate by heading
        }
        return out;
    }
    // Helper to activate motors based on a vector
    bool set_motors(Vector4d v)
    {
        for (int m = static_cast<int>(MotorLocation::FrontRight); m <= static_cast<int>(MotorLocation::BackLeft); m++)
        {
            all_motors[m]->setSpeed(v[m]);
        }
        return true; // TODO error checking?
    }
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
