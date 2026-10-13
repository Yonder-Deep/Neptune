#pragma once
#include <array>

#include <Eigen/Dense>

using namespace Eigen;
/*
TODO: this is a placeholder until the commit w/ imu gets made

*/
class Compass
{
public:
    /**
     * Filler sim version, should convert to abstract once we are done with the IMU
     * Returns the true north vector in the context of the world
     */
    std::array<double, 3> get()
    {
        return {0,0,0};
    }
};
