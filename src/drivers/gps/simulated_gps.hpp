#pragma once
#include "../../types/GPSCoordinate.hpp"
#include "gps.hpp"
#include <cstdio>
#include <httplib.h>

class Simulated_GPS : public GPS {
public:
  const std::string SIM_ADDR = "http://host.docker.internal:6767";
  static httplib::Client *sim_connection;
  Simulated_GPS() {
    if (Simulated_GPS::sim_connection == 0) {
      Simulated_GPS::sim_connection = new httplib::Client(SIM_ADDR);
      httplib::Result out = Simulated_GPS::sim_connection->Get("/heartbeat");
      if (out == nullptr) {
        throw std::runtime_error("No simulation connection found");
      }
      std::cout << "Status code from sim:" << out->status << std::endl;
    }
  }
  bool await_lock(int interval, int tries) override { return true; }
  GPSCoordinate *location() override {
    auto out = Simulated_GPS::sim_connection->Get("/gps");
    if (out == nullptr) {
      std::cout << "Error getting GPS sim info" << std::endl;
      return nullptr;
    }
    GPSCoordinate *s = new GPSCoordinate();
    sscanf(out->body.c_str(), "[%lf,%lf,", &s->latitude, &s->longitude);
    return s;
  }
};
httplib::Client *Simulated_GPS::sim_connection = nullptr;
