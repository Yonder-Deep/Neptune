#pragma once
#include <string>

#include <httplib.h>


const std::string SIM_ADDR = "http://host.docker.internal:6767";
inline httplib::Client *sim_connection = nullptr;
