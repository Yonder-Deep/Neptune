#pragma once
#include "../../types/GPSCoordinate.hpp"
#include <iostream>
/**
 * An abstract class that represents a gps that can gather cordinate location
 */
class GPS {
public:

  virtual bool await_lock(int interval, int tries) {
    std::cout << "Dont call this" << std::endl;
    throw std::logic_error("Tried to use a gps parent class");
  }
  virtual GPSCoordinate location() {
    std::cout << "Dont call this" << std::endl;
    throw std::logic_error("Tried to use a gps parent class");
  }
};
