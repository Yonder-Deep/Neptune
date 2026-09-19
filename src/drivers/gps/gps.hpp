#pragma once
#include "../../types/GPSCoordinate.hpp"
#include <iostream>
/**
 * An abstract class that represents a gps that can gather coordinate location
 */
class GPS {
public:

  virtual bool await_lock(int interval, int tries) = 0;
  virtual GPSCoordinate location() = 0;
};
