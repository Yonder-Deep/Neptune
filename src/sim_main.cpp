#include "drivers/gps/gps.hpp"
#include "drivers/gps/simulated_gps.hpp"
int main() {
  GPS *g = new Simulated_GPS();

  while (true) {
    std::cout << *g->location() << std::endl;
    sleep(1);
  }
}
