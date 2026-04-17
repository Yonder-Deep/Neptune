// main.cpp
#include <iostream>
#include <csignal>
#include <atomic>
#include <memory>

#include "../core/config.hpp"
#include "../core/logger.hpp"
#include "../core/time_utils.hpp"
#include "../core/types.hpp"
#include "../tasks/ws_server.hpp"

using namespace neptune;

// Global flag for graceful shutdown
std::atomic<bool> should_shutdown{false};

// Signal handler for Ctrl+C and termination signals
void signal_handler(int signum) {
    log_info(LogSource::MAIN, "Received signal " + std::to_string(signum) + ", initiating graceful shutdown...");
    should_shutdown.store(true);
}

int main(int argc, char* argv[]) {
    try {
        // Determine config path from command line or use default
        std::string config_path = (argc > 1) ? argv[1] : "data/config.yaml";

        // Step 1: Load configuration from YAML
        log_info(LogSource::MAIN, "Loading configuration from " + config_path);
        ::Config config = ::Config::load(config_path);
        log_info(LogSource::MAIN, "Configuration loaded successfully");

        // Step 2: Initialize logger
        // Generate timestamped log filename: neptune_YYYY-MM-DD_HH-MM-SS.log
        std::string log_filename = timestamped_filename("neptune", "logs", "log");

        // Console level: Info and above
        // File level: Debug and above (more verbose logging to file)
        log_init(log_filename, LogLevel::Info, LogLevel::Debug);
        log_info(LogSource::MAIN, "Logger initialized");

        // Log system configuration
        log_info(LogSource::MAIN, 
                 "System mode - Simulation: " + std::string(config.system.simulation ? "enabled" : "disabled") + 
                 ", Perception: " + std::string(config.system.perception ? "enabled" : "disabled"));
        log_info(LogSource::MAIN, 
                 "Network configured - IP: " + config.network.ip + 
                 ", Port: " + std::to_string(config.network.port));

        // Step 3: Register signal handlers for graceful shutdown
        std::signal(SIGINT, signal_handler);   // Handle Ctrl+C
        std::signal(SIGTERM, signal_handler);  // Handle termination signal

        // Step 4: Create and initialize WebSocket server task
        log_info(LogSource::MAIN, "Initializing WebSocket server on " + config.network.ip + 
                 ":" + std::to_string(config.network.port));
        auto ws_server = std::make_unique<WebSocketServerTask>(config.network.ip, config.network.port);

        // Set up WebSocket message handler
        ws_server->set_message_handler([&config](const std::string& message) {
            log_debug(LogSource::WSKT, "Received message: " + message);
            // TODO: Route message to appropriate subsystem controller
        });

        // Step 5: Start WebSocket server thread
        log_info(LogSource::MAIN, "Starting WebSocket server thread");
        ws_server->startup();
        
        // Activate the WebSocket server to begin accepting connections
        ws_server->activate();
        log_info(LogSource::MAIN, "WebSocket server is now running");

        // Register WebSocket broadcast callback so all logs are sent to connected clients
        // The callback receives a serialized protobuf Envelope
        set_log_broadcast_callback([&ws_server](const std::string& serialized_envelope) {
            ws_server->broadcast(serialized_envelope);
        });
        log_info(LogSource::MAIN, "Log broadcast to WebSocket clients enabled");

        // Step 6: Main control loop
        log_info(LogSource::MAIN, "Entering main control loop");
        RateLoopTimer rate_timer(config.control.loop_hz);


        while (!should_shutdown.load()) {
            // Mark the start of this cycle
            rate_timer.tick();

            // TODO: Implement main control logic here
            // - Read sensor data and update vehicle_state
            // - Run estimators (localization) with vehicle_state
            // - Execute control algorithms based on vehicle_state
            // - Update motor commands based on control outputs
            // - Send telemetry to WebSocket clients (vehicle_state, control outputs, etc.)

            // Example: Log vehicle state periodically (placeholder)
            // log_debug(LogSource::MAIN, 
            //          "Position [N,E,D]: " + std::to_string(vehicle_state.position[0]) + 
            //          ", " + std::to_string(vehicle_state.position[1]) + 
            //          ", " + std::to_string(vehicle_state.position[2]));

            log_info(LogSource::MAIN, "heartbeat");
            // Wait until the next cycle is due, maintaining the desired control loop rate
            double cycle_time = rate_timer.wait();
            if (rate_timer.overran()) {
                log_warn(LogSource::MAIN, 
                         "Control loop overran desired period (took " + std::to_string(cycle_time) + " seconds)");
            }
        }

        // Step 7: Graceful shutdown sequence
        log_info(LogSource::MAIN, "Shutting down main control system");

        // Disable WebSocket broadcast before shutting down server
        clear_log_broadcast_callback();

        log_info(LogSource::MAIN, "Deactivating WebSocket server");
        ws_server->deactivate();

        log_info(LogSource::MAIN, "Stopping WebSocket server");
        ws_server->shutdown();

        log_info(LogSource::MAIN, "Waiting for WebSocket server thread to finish");
        ws_server->join();

        log_info(LogSource::MAIN, "All tasks completed successfully");

        // Step 8: Shutdown logger (flush logs to disk)
        log_shutdown();

        return 0;

    } catch (const std::exception& e) {
        // Log the exception before shutdown if logger is initialized
        log_critical(LogSource::MAIN, std::string("Fatal error: ") + e.what());
        log_shutdown();
    
        return 1;
    }
}
