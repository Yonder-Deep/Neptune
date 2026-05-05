#include "sim_motor.hpp"
#include <sstream>
httplib::Client *SimulatedMotor::sim_connection = nullptr;
std::string SimulatedMotor::GetMotorPath() {
  switch (this->ml) {
  case SimulatedMotor::FrontLeft:
    return "/motor/top_left/set";
    break;
  case SimulatedMotor::FrontRight:
    return "/motor/top_right/set";
    break;
  case SimulatedMotor::BackLeft:
    return "/motor/bottom_left/set";
    break;
  case SimulatedMotor::BackRight:
    return "/motor/bottom_right/set";
    break;
  default:
    // Error?
    break;
  }
  return "";
}

SimulatedMotor::SimulatedMotor(SimulatedMotor::MotorLocation ml, int speed,
                               float cycle)
    : Motor(speed, cycle) {
  if (SimulatedMotor::sim_connection == 0) {
    SimulatedMotor::sim_connection = new httplib::Client(SIM_ADDR);
    httplib::Result out = SimulatedMotor::sim_connection->Get("/heartbeat");
    if (out == nullptr) {
      throw std::runtime_error("No simulation connection found");
    }
    std::cout << "Status code from sim:" << out->status << std::endl;
  }
  this->ml = ml;
}

int SimulatedMotor::setSpeed(float speed) {
  if (!this->armed) {
    throw std::logic_error("Tried to set the speed of an unarmed motor");
  }
  this->speed = speed;
  // In the long run we should add a json lib
  std::ostringstream oss;
  oss << "{\"speed\":" << speed << "}";
  std::string s = oss.str();
  auto out = SimulatedMotor::sim_connection->Post(
      this->GetMotorPath().c_str(), s.c_str(), "application/json");
  if (out == nullptr) {
    std::cout << "Error sending sim info" << std::endl;
  } else {
    std::cout << out->status << std::endl;
  }

  return 0;
}

int SimulatedMotor::setFrequency(float cycle) {
  // idk if this will be called at all
  return 0;
}

void SimulatedMotor::armMotor() { this->armed = true; }
