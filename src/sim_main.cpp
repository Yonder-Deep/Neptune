#include "drivers/gps/gps.hpp"
#include "drivers/gps/simulated_gps.hpp"
#include "drivers/motors/sim_motor.hpp"
#include "drivers/motors/motor.hpp"
#include "types/MotorLocation.hpp"
#include "neptune.hpp"
#include "states/navigate.hpp"
#include "states/menu.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main()

{
  sim_connection = new httplib::Client(SIM_ADDR);
  GPS *g = new Simulated_GPS();

  Motor *front_left = new SimulatedMotor(MotorLocation::FrontLeft, 0, 50);
  Motor *front_right = new SimulatedMotor(MotorLocation::FrontRight, 0, 50);
  Motor *back_left = new SimulatedMotor(MotorLocation::BackLeft, 0, 50);
  Motor *back_right = new SimulatedMotor(MotorLocation::BackRight, 0, 50);
  Neptune *neptune = new Neptune();
  neptune->gps = g;
  neptune->back_left = back_left;
  neptune->back_right = back_right;
  neptune->front_left = front_left;
  neptune->front_right = front_right;
  neptune->compass = new Compass();
  neptune->state = new Menu();
  neptune->start();

  return 0;
}
