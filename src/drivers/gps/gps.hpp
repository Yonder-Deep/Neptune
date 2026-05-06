#pragma once
#include "../../types/GPSCoordinate.hpp"
#include <iostream>
/**
 */
class GPS {
public:
  virtual bool await_lock(int interval, int tries) {
    std::cout << "Dont call this" << std::endl;
    return false;
  }
  virtual GPSCoordinate *location() {
    std::cout << "Dont call this" << std::endl;
    return nullptr;
  }
};
