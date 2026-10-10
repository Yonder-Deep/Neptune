#pragma once
#include "motor.hpp"
#include <httplib.h>
#include "../../types/MotorLocation.hpp"

class SimulatedMotor : public Motor
{

public:
  MotorLocation ml;
  bool armed;

  SimulatedMotor(MotorLocation ml, int speed,
                                 float cycle) : Motor(speed, cycle)
  {
    check_sim_connected();
    this->ml = ml;
  }
  int setSpeed(float speed) override {
  if (!this->armed) {
    throw std::logic_error("Tried to set the speed of an unarmed motor");
  }
  this->speed = speed;
  // In the long run we should add a json lib
  std::ostringstream oss;
  oss << "{\"speed\":" << speed << "}";
  std::string s = oss.str();
  send_sim_msg(this->GetMotorPath(), s, HTTPMethod::Post );
  return 0;
}
int setFrequency(float cycle) override {
  // idk if this will be called at all
  return 0;
}

void armMotor()override { this->armed = true; }


private:
  std::string GetMotorPath()
  {
    switch (this->ml)
    {
    case FrontLeft:
      return "/motor/top_left/set";
      break;
    case FrontRight:
      return "/motor/top_right/set";
      break;
    case BackLeft:
      return "/motor/bottom_left/set";
      break;
    case BackRight:
      return "/motor/bottom_right/set";
      break;
    default:
      // Error?
      break;
    }
    return "";
  }
};
