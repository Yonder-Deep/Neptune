// main.cpp
#include "CLI11.hpp"
#include "drivers/gps/hardware_gps.hpp"
int main(int argc, char **argv) {
  CLI::App app;
  std::string gps_port;
  app.add_option("--gps", gps_port,
                 "The string path to the gps. Likely, /dev/ttyACM0")
      ->required();
  CLI11_PARSE(app, argc, argv);
  GPS *gps = new HardwareGPS(gps_port);
  std::cout << "awaiting lock" << std::endl;
  gps->await_lock(5, 200);
  while (true) {
    std::cout << *(gps->location()) << std::endl;
    sleep(3);
  }
}
