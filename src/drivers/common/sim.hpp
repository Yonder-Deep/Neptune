#pragma once

#include <stdexcept>
#include <string>
#include <httplib.h>

// Blank namespace is used to make these private. Dunno why c++ doesn't allow private outside of classes
namespace
{
    inline const std::string SIM_ADDR = "http://host.docker.internal:6767";
    inline httplib::Client *sim_connection = nullptr;
}
enum HTTPMethod
{
    Post,
    Get
};

/**
 * Sends a message to the sim.
 * @param route Which path to send the request to. Should start with a forward slash
 * @param msg The body of the request, for post requests. Should be a json payload
 * @param method HTTP method
 * @throws runtime_error: When the sim is not able to connect
 * @returns The httplib result of the request
 */
inline httplib::Result send_sim_msg(const std::string &route, const std::string &msg = "", HTTPMethod method = HTTPMethod::Get)
{
    if (sim_connection == nullptr)
    {
        sim_connection = new httplib::Client(SIM_ADDR);
    }
    httplib::Result out = (method == HTTPMethod::Post)
                              ? sim_connection->Post(route, msg, "application/json")
                              : sim_connection->Get(route);
    if (out == nullptr)
    {
        throw std::runtime_error("No simulation connection found, are you sure it is running?");
    }
    return out;
}

inline bool check_sim_connected()
{
    try
    {
        send_sim_msg("/heartbeat", "");
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}
