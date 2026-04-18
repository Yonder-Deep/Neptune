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
        std::string config_path = (argc > 1) ? argv[1] : "~/Neptune/auv_cpp/data/config.yaml";

        std::string log_filename = timestamped_filename("neptune", "logs", "log");

        ::Config config = ::Config::load(config_path);

        // Console level: Info and above, File level: Debug and above (more verbose logging to file)
        log_init(log_filename, LogLevel::Info, LogLevel::Debug);

        
        auto ws_server = std::make_unique<WebSocketServerTask>(config.network.ip, config.network.port);
        ws_server->startup();
        ws_server->activate();
        

        // Set main thread's task context for logging 
        // Since this is the main control loop and not technically a thread_task, 
        // use the WebSocket server task's context for logging so we can enqueue 
        // messages to its outbound SPSC queue instead of having a separate logger thread.
        set_current_task(ws_server.get());


        log_info(LogSource::MAIN, "Logger initialized");



        // Load configuration from YAML
        log_info(LogSource::MAIN, "Loaded configuration from " + config_path);
    
        // Log system configuration
        log_info(
            LogSource::MAIN, 
            "System mode - Simulation: " + std::string(config.system.simulation ? "enabled" : "disabled") + 
            ", Perception: " + std::string(config.system.perception ? "enabled" : "disabled")
        );
        log_info(
            LogSource::MAIN, 
            "Network configured - IP: " + config.network.ip + 
            ", Port: " + std::to_string(config.network.port)
        );



        // Register signal handlers for graceful shutdown
        std::signal(SIGINT, signal_handler);   // Handle Ctrl+C
        std::signal(SIGTERM, signal_handler);  // Handle termination signal




        // Declare all other tasks here, add to the vector so we can manage them in the main loop
        // auto motor_control, etc

        
        // Add more tasks to this vector as they're created
        std::vector<ThreadTask*> task_list = {ws_server.get()}; 

        for (auto* task : task_list) {
            if (task->name == "ws-server") {
                continue;
            }
            log_info(LogSource::MAIN, "Starting task: " + task->name);
            task->startup();
            task->activate();
        }

        log_info(LogSource::MAIN, "All tasks started successfully");




        //Main control loop
        log_info(LogSource::MAIN, "Entering main control loop");
        RateLoopTimer rate_timer(config.control.loop_hz);


        while (!should_shutdown.load()) {
            // Mark the start of this cycle
            rate_timer.tick();

            // Poll all task output queues and broadcast messages to base station
            for (auto* task : task_list) {
                while (auto msg = task->outbound_q.pop()) {
                    ws_server->broadcast(std::string(msg->envelope_bytes.data(), msg->envelope_len));
                }
            }
            

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
                log_warn(
                    LogSource::MAIN, 
                    "Control loop overran desired period (took " + std::to_string(cycle_time) + " seconds)"
                );
            }

        }


        

        // Graceful shutdown sequence
        log_info(LogSource::MAIN, "Shutting down main control system");

        // Drain any remaining messages in task queues before shutdown
        for (auto* task : task_list) {
            if (task->name == "ws-server") {
                continue;
            }

            task->deactivate();
            while (auto msg = task->outbound_q.pop()) {
                ws_server->broadcast(std::string(msg->envelope_bytes.data(), msg->envelope_len));
            }
            log_info(LogSource::MAIN, "Drained and deactivated task: " + task->name);
        }

        // flush WS logs, then shutdown server
        while (auto msg = ws_server->outbound_q.pop()) {
            ws_server->broadcast(std::string(msg->envelope_bytes.data(), msg->envelope_len));
        }

        log_info(LogSource::MAIN, "Drained and deactivating WebSocket server. Goodbye!");
        ws_server->deactivate();

        for (auto* task : task_list) {
            task->shutdown();
            task->join();
            log_info(LogSource::MAIN, "Shutdown and joined task: " + task->name);
        }



        log_shutdown();

        return 0;

    } 
    catch (const std::exception& e) {
        // Log the exception before shutdown if logger is initialized
        log_critical(LogSource::MAIN, std::string("Fatal error: ") + e.what());
        log_shutdown();
    
        return 1;
    }
}
