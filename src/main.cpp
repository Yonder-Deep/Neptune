// main.cpp
#include "CLI11.hpp"
#include "drivers/gps/hardware_gps.hpp"
#include "drivers/gps/gps.hpp"
#include "drivers/motors/motor.hpp"
#include "neptune.hpp"
#include "states/navigate.hpp"
#include "drivers/motors/thruster.hpp"
#include "states/menu.hpp"
#include "lgpio.h"
#include <iostream>
#include <thread>
#include <chrono>
int main(int argc, char **argv)
{
  CLI::App app;
  std::string gps_port;
  app.add_option("--gps", gps_port,
                 "The string path to the gps. Likely, /dev/ttyACM0")
      ->required();

  std::vector<int> pins;
  app.add_option("--motors", pins, "The 4 pins for motors (space delimited) in the form: front_left front_right back_left back_right");
  CLI11_PARSE(app, argc, argv);
  GPS *gps = new HardwareGPS(gps_port);
  //Hardcoded since I dont think this will ever change on the pi

  if(pins.size() != 4){
    std::cout << "Wrong number of input pins for motors:" << pins.size() <<"\n";
    return 1;
  }
  int handle = lgGpiochipOpen(0);
  Motor *front_left = new Thruster(pins[0], handle);
  Motor *front_right = new Thruster(pins[1], handle);
  Motor *back_left = new Thruster(pins[2], handle);
  Motor *back_right = new Thruster(pins[3], handle);
  Neptune *neptune = new Neptune();
  neptune->gps = gps;
  neptune->back_left = back_left;
  neptune->back_right = back_right;
  neptune->front_left = front_left;
  neptune->front_right = front_right;
  neptune->compass = new Compass();
  neptune->state = new Menu();
  neptune->start();
}
