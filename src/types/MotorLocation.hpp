#pragma once
#include "GPSCoordinate.hpp"
#include <numbers>
enum MotorLocation { FrontRight, FrontLeft, BackRight, BackLeft };

constexpr double SQRT2 = 1.41421356237309504880;
//TODO make sure these stay up to date, and maybe consider a seperate point class that isn't a GPScoord
GPSCoordinate thrust_vector(MotorLocation m){
    switch(m){
        case FrontLeft:
        return {0.5 * SQRT2, 0.5 * SQRT2};
        case FrontRight:
        return {-0.5* SQRT2, 0.5* SQRT2};
        case BackRight:
        return {-0.5* SQRT2, -0.5* SQRT2};
        case BackLeft:
        return {0.5* SQRT2, -0.5* SQRT2};
    }

}
// 0 - lat
// 1 - long
