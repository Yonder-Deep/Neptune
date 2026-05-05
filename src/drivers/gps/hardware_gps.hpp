#include "gps.hpp"

#include "../../types/GPSCoordinate.hpp"
#include <iostream>
#include <libgpsmm.h>
#include <string>
/**
 * Note - its likely that only 1 gps can function at a time
 */
class HardwareGPS : public GPS {
public:
  ~HardwareGPS() { delete this->gps_rec; }
  GPSCoordinate *location() override {
    while (true) {
      struct gps_data_t *data;

      if (!(this->gps_rec->waiting(10000))) {
        continue;
      }

      if ((data = this->gps_rec->read()) == NULL) {
        std::cerr << "Read error." << std::endl;
        // TODO - what to do here?
        return nullptr;
      } else {
        if (data->fix.mode >= MODE_2D) {
          // Connection gotten
          GPSCoordinate *out = new GPSCoordinate();
          out->latitude = data->fix.latitude;
          out->longitude = data->fix.longitude;

          return out;
        } else {
          std::cout << "No gps fix" << std::endl;
        }
      }
    }
  }
  HardwareGPS(std::string location, const char *gpsd_port = DEFAULT_GPSD_PORT) {
    std::string out = "gpsd " + location;
    int val = system(out.c_str());
    if (val != 0) {
      throw new std::runtime_error("GPS failed to connect - gpsd didn't start");
    }
    std::cout << "GPS loaded" << std::endl;
    this->gps_rec = new gpsmm("localhost", gpsd_port);
  }
  bool await_lock(int interval, int tries) override {
    if (this->gps_rec->stream(WATCH_ENABLE | WATCH_JSON) == NULL) {
      std::cerr << "No GPSD running." << std::endl;
      return 1;
    }
    int attempt = 0;
    while (true) {
      struct gps_data_t *data;

      if (!(this->gps_rec->waiting(interval * 10000))) {
        continue;
      }

      if ((data = this->gps_rec->read()) == NULL) {
        std::cerr << "Read error." << std::endl;
        // TODO - what to do here?
        return false;
      } else {
        if (data->fix.mode >= MODE_2D) {
          // Connection gotten
          return true;
        } else {
          if (++attempt > tries) {
            return false;
          }
          std::cout << "No gps fix" << std::endl;
        }
      }
    }
    return true;
  }

private:
  gpsmm *gps_rec;
};