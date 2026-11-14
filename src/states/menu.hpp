#pragma once
#include "state.hpp"
#include "../neptune.hpp"
#include <optional>
#include <array>
#include <cmath>
#include <iostream>

/**
 * Rep
 */
class Menu : public State
{
public:
    bool finished;
    int wait = 500;
    double speed = 0.5;
    Menu()
    {
    }

    std::chrono::milliseconds tick() override
    {
        Neptune::instance->stop();
        std::cout << "Current duration(ms): " << wait << "Current Speed:" << speed << "\nOptions:\n[0] Rotate Clockwise \n[1] Rotate Counter-Clockwise \n[2] Move Forwards \n[3] Move Backwards \n[4]Set Duration \n[5]Change Speed \n[6]Read Sensors \n[7] Strafe Right \n[8] Strafe Left \n[9] Forward-Right + Rotate CW" << std::endl;
        std::cout << ":";
        int out;
        while (!(std::cin >> out) && (0 <= out) && (out <= 6))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number: ";
        }
        switch (out)
        {
        case 0:
            Neptune::instance->drive({0, 0, this->speed});
            return std::chrono::milliseconds(this->wait);
        case 1:
            Neptune::instance->drive({0, 0, -this->speed});
            return std::chrono::milliseconds(this->wait);
        case 2:
            Neptune::instance->drive({this->speed, 0, 0});
            return std::chrono::milliseconds(this->wait);
        case 3:
            Neptune::instance->drive({-this->speed, 0, 0});
            return std::chrono::milliseconds(this->wait);
        case 4:
            int wait;
            std::cout << "New duration(ms):";
            while (!(std::cin >> wait))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter a number: ";
            }
            this->wait = wait;
            return std::chrono::milliseconds(10);
        case 5:
            double speed;
            std::cout << "New speed(0.01 to 1):";
            while (!(std::cin >> speed) && (0.01 <= speed) && (speed <= 1))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter a number: ";
            }
            this->speed = speed;
            return std::chrono::milliseconds(10);
        case 6:
            Neptune::instance->dispData();
            return std::chrono::milliseconds(10);
        case 7:
            Neptune::instance->drive({0, this->speed, 0});
            return std::chrono::milliseconds(this->wait);
        case 8:
            Neptune::instance->drive({0, -this->speed, 0});
            return std::chrono::milliseconds(this->wait);
        case 9:
            Neptune::instance->drive({this->speed, this->speed, this->speed});
            return std::chrono::milliseconds(this->wait);
        }
        return std::chrono::milliseconds(wait);
    }
};
