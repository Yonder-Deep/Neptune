// log_client.cpp
// WebSocket client that receives protobuf-serialized log messages from Neptune AUV
// Connects to 127.0.0.1:8080 and deserializes/displays log messages

#include <iostream>
#include <string>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <iomanip>
#include <ctime>

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

#include "messages.pb.h"  // Protobuf messages

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::client<websocketpp::config::asio_client> Client;
typedef websocketpp::config::asio_client::message_type::ptr MessagePtr;
typedef websocketpp::connection_hdl ConnectionHandle;

// Global state
std::atomic<bool> should_exit{false};
Client client;
ConnectionHandle connection;
std::atomic<bool> connected{false};

// Signal handler for graceful shutdown
void signal_handler(int signum) {
    std::cout << "\n[SIGNAL] Received signal " << signum << ", shutting down..." << std::endl;
    should_exit.store(true);
}

// Callback when connection opens
void on_open(Client* c, ConnectionHandle hdl) {
    connection = hdl;
    connected.store(true);
    std::cout << "[CLIENT] Connected to server" << std::endl;
}

// Callback when connection closes
void on_close(Client* c, ConnectionHandle hdl) {
    connected.store(false);
    std::cout << "[CLIENT] Disconnected from server" << std::endl;
}

// Callback when a message is received
void on_message(Client* c, ConnectionHandle hdl, MessagePtr msg) {
    try {
        // Get the binary payload
        std::string payload = msg->get_payload();

        // Deserialize protobuf Envelope
        neptune::Envelope envelope;
        if (!envelope.ParseFromString(payload)) {
            std::cerr << "[ERROR] Failed to parse Envelope protobuf" << std::endl;
            return;
        }

        // Handle different message types
        switch (envelope.type()) {
            case neptune::Envelope::LOG: {
                // Deserialize LogEntry from envelope payload
                neptune::LogEntry log_entry;
                if (!log_entry.ParseFromString(envelope.payload())) {
                    std::cerr << "[ERROR] Failed to parse LogEntry protobuf" << std::endl;
                    return;
                }

                // Format timestamp
                auto time_ms = log_entry.timestamp_ms();
                auto time_t = time_ms / 1000;
                auto ms = time_ms % 1000;
                std::tm* timeinfo = std::localtime(&time_t);
                
                char time_buf[32];
                std::strftime(time_buf, sizeof(time_buf), "%H:%M:%S", timeinfo);

                // Print formatted log
                std::cout << "[" << std::setfill('0') << std::setw(2) << timeinfo->tm_hour
                          << ":" << std::setfill('0') << std::setw(2) << timeinfo->tm_min
                          << ":" << std::setfill('0') << std::setw(2) << timeinfo->tm_sec
                          << "." << std::setfill('0') << std::setw(3) << ms << "] "
                          << "[" << log_entry.level() << "] "
                          << "[" << log_entry.source() << "] "
                          << log_entry.message() << std::endl;
                break;
            }

            case neptune::Envelope::TELEMETRY: {
                neptune::Telemetry telemetry;
                if (!telemetry.ParseFromString(envelope.payload())) {
                    std::cerr << "[ERROR] Failed to parse Telemetry protobuf" << std::endl;
                    return;
                }
                std::cout << "[TELEMETRY] Received vehicle telemetry update" << std::endl;
                break;
            }

            case neptune::Envelope::COMMAND_RESPONSE: {
                neptune::CommandResponse response;
                if (!response.ParseFromString(envelope.payload())) {
                    std::cerr << "[ERROR] Failed to parse CommandResponse protobuf" << std::endl;
                    return;
                }
                std::cout << "[COMMAND] Command #" << response.command_id() 
                          << " " << (response.success() ? "succeeded" : "failed")
                          << ": " << response.message() << std::endl;
                break;
            }

            default:
                std::cerr << "[WARNING] Unknown message type: " << envelope.type() << std::endl;
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception processing message: " << e.what() << std::endl;
    }
}

// Callback on error
void on_fail(Client* c, ConnectionHandle hdl) {
    std::cout << "[ERROR] Connection failed" << std::endl;
    connected.store(false);
}

int main() {
    try {
        // Setup signal handlers
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        // Initialize client
        client.set_access_channels(websocketpp::log::alevel::all);
        client.clear_access_channels(websocketpp::log::alevel::frame_header |
                                      websocketpp::log::alevel::frame_payload);
        client.set_error_channels(websocketpp::log::elevel::all);
        client.init_asio();

        // Set connection callbacks
        client.set_open_handler(bind(&on_open, &client, ::_1));
        client.set_close_handler(bind(&on_close, &client, ::_1));
        client.set_message_handler(bind(&on_message, &client, ::_1, ::_2));
        client.set_fail_handler(bind(&on_fail, &client, ::_1));

        // Connect to server
        std::string uri = "ws://127.0.0.1:8080";
        std::cout << "[CLIENT] Connecting to " << uri << "..." << std::endl;

        websocketpp::lib::error_code ec;
        Client::connection_ptr con = client.get_connection(uri, ec);
        if (ec) {
            std::cerr << "[ERROR] Connection initialization error: " << ec.message() << std::endl;
            return 1;
        }

        client.connect(con);

        // Run client in a separate thread
        std::thread client_thread([&]() {
            try {
                client.run();
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Client exception: " << e.what() << std::endl;
            }
        });

        // Main thread: wait for shutdown signal or connection loss
        while (!should_exit.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Graceful shutdown
        std::cout << "[CLIENT] Initiating graceful shutdown..." << std::endl;

        if (connected.load()) {
            client.close(connection, websocketpp::close::status::normal, "client shutdown");
        }

        // Stop the client (may take a moment to close connections)
        client.stop();

        // Wait for client thread to finish
        if (client_thread.joinable()) {
            client_thread.join();
        }

        std::cout << "[CLIENT] Shutdown complete" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] Exception: " << e.what() << std::endl;
        return 1;
    }
}
