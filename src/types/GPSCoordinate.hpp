#pragma once

#include <ostream>

struct GPSCoordinate {
  double latitude;
  double longitude;
};

// Equality operators
inline bool operator==(GPSCoordinate const& lhs, GPSCoordinate const& rhs) {
    return lhs.latitude == rhs.latitude && lhs.longitude == rhs.longitude;
}

inline bool operator!=(GPSCoordinate const& lhs, GPSCoordinate const& rhs) {
    return !(lhs == rhs);
}

// Comparison operators (lexicographical)
inline bool operator<(GPSCoordinate const& lhs, GPSCoordinate const& rhs) {
    if (lhs.latitude != rhs.latitude) return lhs.latitude < rhs.latitude;
    return lhs.longitude < rhs.longitude;
}

inline bool operator>(GPSCoordinate const& lhs, GPSCoordinate const& rhs) {
    return rhs < lhs;
}

inline bool operator<=(GPSCoordinate const& lhs, GPSCoordinate const& rhs) {
    return !(rhs < lhs);
}

inline bool operator>=(GPSCoordinate const& lhs, GPSCoordinate const& rhs) {
    return !(lhs < rhs);
}

// Arithmetic operators
inline GPSCoordinate operator+(GPSCoordinate const& lhs, GPSCoordinate const& rhs) {
    return {lhs.latitude + rhs.latitude, lhs.longitude + rhs.longitude};
}

inline GPSCoordinate operator-(GPSCoordinate const& lhs, GPSCoordinate const& rhs) {
    return {lhs.latitude - rhs.latitude, lhs.longitude - rhs.longitude};
}

inline GPSCoordinate operator*(GPSCoordinate const& coord, double scalar) {
    return {coord.latitude * scalar, coord.longitude * scalar};
}

inline GPSCoordinate operator*(double scalar, GPSCoordinate const& coord) {
    return coord * scalar;
}

inline GPSCoordinate operator/(GPSCoordinate const& coord, double scalar) {
    return {coord.latitude / scalar, coord.longitude / scalar};
}

// Unary minus
inline GPSCoordinate operator-(GPSCoordinate const& coord) {
    return {-coord.latitude, -coord.longitude};
}

// Compound assignment
inline GPSCoordinate& operator+=(GPSCoordinate& lhs, GPSCoordinate const& rhs) {
    lhs.latitude += rhs.latitude;
    lhs.longitude += rhs.longitude;
    return lhs;
}

inline GPSCoordinate& operator-=(GPSCoordinate& lhs, GPSCoordinate const& rhs) {
    lhs.latitude -= rhs.latitude;
    lhs.longitude -= rhs.longitude;
    return lhs;
}

inline GPSCoordinate& operator*=(GPSCoordinate& coord, double scalar) {
    coord.latitude *= scalar;
    coord.longitude *= scalar;
    return coord;
}

inline GPSCoordinate& operator/=(GPSCoordinate& coord, double scalar) {
    coord.latitude /= scalar;
    coord.longitude /= scalar;
    return coord;
}

inline std::ostream &operator<<(std::ostream &os, GPSCoordinate const &coord) {
  return os << "GPSCoordinate{" << coord.latitude << ", " << coord.longitude
            << "}";
}
