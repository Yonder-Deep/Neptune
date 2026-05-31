#pragma once
#include "GPSCoordinate.hpp"
#include <numbers>

#include <Eigen/Dense>
using namespace Eigen;

//IMPORTANT: If the robot is placed facing north, the front motors should be facing north
enum MotorLocation { FrontRight, FrontLeft, BackRight, BackLeft };

constexpr double SQRT2 = 1.41421356237309504880;
//TODO make sure these stay up to date
Vector2d thrust_vector(MotorLocation m){
    switch(m){
        case FrontLeft:
        return {0.5 * SQRT2, 0.5 * SQRT2};
        case FrontRight:
        return {0.5* SQRT2, -0.5* SQRT2};
        case BackRight:
        return {-0.5* SQRT2, 0.5* SQRT2};
        case BackLeft:
        return {-0.5* SQRT2, -0.5* SQRT2};
    }
    throw std::logic_error("Invalid type");
}
