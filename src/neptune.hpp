#pragma once
#include "drivers/motors/motor.hpp"
#include "drivers/gps/gps.hpp"
#include <cstdlib>
#include <unistd.h>
#include "states/state.hpp"

class Neptune {
    public:
    State* state;
    inline static Neptune* instance = nullptr;
    GPS* gps;
    Motor* front_left;
    Motor* front_right;
    Motor* back_left;
    Motor* back_right;
    Motor* all_motors[4];
    /**
     * Gets the heading, in radians of the robot. 0 indicates north, pi indicates south
     * @return The heading
     */
    double get_heading(){
        //TODO - Use IMU to calculate this
        return 0;
    }
    std::array<std::array<double, 5>, 2> GetMotorVectorMatrice() {
         std::array<std::array<double, 5>, 2>  out = {
         };
        for(int m =  0; m <= 3;  m ++) {
            out[0][m] = thrust_vector(static_cast<MotorLocation>(m)).latitude;
            out[1][m] = thrust_vector(static_cast<MotorLocation>(m)).longitude;
        }
        return out;
     }
    /**
     * Starts neptune. This method will functionally take over the thread it is called from
     */
    void start(){
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

};
