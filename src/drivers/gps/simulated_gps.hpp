#pragma once
#include "../../types/GPSCoordinate.hpp"
#include "gps.hpp"
#include <cstdio>
#include "../common/sim.hpp"

class Simulated_GPS : public GPS {
public:

  Simulated_GPS() {
    check_sim_connected();
  }
  bool await_lock(int interval, int tries) override { return true; }
  GPSCoordinate location() override {
    auto out = send_sim_msg("/gps", "");
    GPSCoordinate s =  GPSCoordinate();
    sscanf(out->body.c_str(), "[%lf,%lf,", &s.latitude, &s.longitude);
    return s;
  }
};
