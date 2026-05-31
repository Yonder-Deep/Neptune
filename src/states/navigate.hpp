#pragma once
#include "state.hpp"
#include "../../neptune.hpp"
#include <optional>
#include <array>
#include <cmath>
#include <iostream>

class Navigate : public State
{
public:
    bool finished;
    GPSCoordinate goal;
    Navigate(GPSCoordinate goal) : goal(goal)
    {
    }

    std::chrono::milliseconds tick() override
    {
        //std::cout << Neptune::instance->get_heading() << std::endl;
        Neptune::instance->travel(goal);
        return std::chrono::milliseconds(50);
    }
};
