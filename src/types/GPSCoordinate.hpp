#pragma once

#include <ostream>

struct GPSCoordinate {
    double latitude;
    double longitude;
};

inline std::ostream& operator<<(std::ostream& os, GPSCoordinate const& coord) {
    return os << "GPSCoordinate{" << coord.latitude << ", " << coord.longitude << "}";
}
