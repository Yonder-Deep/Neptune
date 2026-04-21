#include <iostream>
#include <lgpio.h>
#include <thread>
#include "motor.hpp"
#include "thruster.hpp"

int main()
{
  int handle = lgGpiochipOpen(0);
  Motor* sm1 = new Thruster(18, handle);
  sm1->armMotor();

  sm1->setSpeed(0.3);
  lguSleep(1.0);
  sm1->setSpeed(-0.3);
  lguSleep(1.0);
  sm1->setSpeed(0.0);
  // Motor* sm1 = new SimulatedMotor(SimulatedMotor::BackLeft);
  // Motor* sm2 = new SimulatedMotor(SimulatedMotor::BackRight);
  // Motor* sm3 = new SimulatedMotor(SimulatedMotor::FrontLeft);
  // Motor* sm4 = new SimulatedMotor(SimulatedMotor::FrontRight);
  // sm1->armMotor();
  // sm2->armMotor();
  // sm3->armMotor();
  // sm4->armMotor();
  // sm1->setSpeed(1);
  // sm2->setSpeed(0);
  // sm3->setSpeed(0);
  // sm4->setSpeed(0);

  
}
