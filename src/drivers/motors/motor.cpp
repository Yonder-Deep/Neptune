#include "motor.hpp"
#include <iostream>

// All of these should probably not be here or throw execptions but I need to
// put them here for the linker
int Motor::setSpeed(float speed) {
  std::cout << "Dont call me" << std::endl;
  return 0;
}

int Motor::setFrequency(float cycle) {
  std::cout << "Dont call me" << std::endl;
  return 0;
}

void Motor::armMotor() {}
