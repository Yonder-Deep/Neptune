#pragma once
#include "state.hpp"
#include "../../neptune.hpp"
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
        std::cout << "Current duration(ms): " << wait << "Current Speed:" << speed << "\nOptions:\n[0] Rotate Clockwise \n[1] Rotate Counter-Clockwise \n[2] Move Forwards \n[3] Move Backwards \n[4]Set Duration \n[5]Change Speed \n[6]Read Sensors" << std::endl;
        std::cout << ":";
        int out;
        while (!(std::cin >> out) && (0 <= out) && (out <= 6))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number: ";
        }
        std::cout << "running:" << out << std::endl;
        switch (out)
        {
        case 0:
            Neptune::instance->rotate(true, speed);
            return std::chrono::milliseconds(wait);
        case 1:
            Neptune::instance->rotate(true, speed);
            return std::chrono::milliseconds(wait);
        case 2:
            Neptune::instance->forwards(speed);
            return std::chrono::milliseconds(wait);
        case 3:
            Neptune::instance->backwards(speed);
            return std::chrono::milliseconds(wait);
        case 4:
            int wait;
            while (!(std::cin >> wait))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter a number: ";
            }
            this->wait = wait;
            return std::chrono::milliseconds(wait);
        case 5:
            double speed;
            while (!(std::cin >> speed) && (0.01 <= speed) && (speed <= 1))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter a number: ";
            }
            this->speed = speed;
            return std::chrono::milliseconds(wait);
        case 6:
            Neptune::instance->dispData();
            return std::chrono::milliseconds(wait);
        }
        return std::chrono::milliseconds(wait);
    }
};
