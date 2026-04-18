#pragma once
#include <memory>
#include <set>
#include <string>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "../core/logger.hpp"
#include "thread_task.hpp"

using namespace neptune;

// WebSocket++ server typedef.
// Uses ASIO (no TLS), which allows async I/O without blocking.
using WSServer = websocketpp::server<websocketpp::config::asio>;
using ConnectionHandle = websocketpp::connection_hdl;

// WebSocket server thread that manages client connections and message routing.
//
// Lifecycle:
//   1. Constructor sets endpoint and handlers
//   2. startup()    — spawns the thread (blocked)
//   3. activate()   — starts listening on the specified endpoint
//   4. deactivate() — stops accepting new connections
//   5. shutdown()   — closes all connections and stops the server
//   6. join()       — waits for the thread to finish
//
// Usage:
//   WebSocketServerTask ws_server("0.0.0.0", 8080);
//   ws_server.startup();
//   ws_server.activate();
//   // ... messages are routed through on_message callback ...
//   ws_server.deactivate();
//   ws_server.shutdown();
//   ws_server.join();
//
class WebSocketServerTask : public ThreadTask {
public:
    explicit WebSocketServerTask(const std::string& ip, uint16_t port)
        : ThreadTask("ws-server"), ip_(ip), port_(port), endpoint_() {
        configure_endpoint();
    }

    ~WebSocketServerTask() override = default;

    // Prevent copying
    WebSocketServerTask(const WebSocketServerTask&) = delete;
    WebSocketServerTask& operator=(const WebSocketServerTask&) = delete;

    // Register a callback to handle incoming messages.
    // Callback signature: void(const std::string& message)
    template <typename Callback>
    void set_message_handler(Callback&& cb) {
        message_handler_ = [cb](const std::string& msg) { cb(msg); };
    }

    // Send a message to all connected clients.
    // Uses binary frame opcode for protobuf payloads
    void broadcast(const std::string& message) {
        {
            std::lock_guard<std::mutex> lock(connections_mtx_);
            for (auto& handle : connections_) {
                try {
                    endpoint_.send(handle, message, websocketpp::frame::opcode::binary);
                } catch (const std::exception& e) {
                    log_error(LogSource::WSKT, std::string("broadcast error: ") + e.what());
                }
            }
        }
    }

    // Send a message to a specific client by handle.
    // Uses binary frame opcode for protobuf payloads
    void send_to(ConnectionHandle handle, const std::string& message) {
        try {
            endpoint_.send(handle, message, websocketpp::frame::opcode::binary);
        } catch (const std::exception& e) {
            log_error(LogSource::WSKT, std::string("send error: ") + e.what());
        }
    }

    // Get the number of currently connected clients.
    size_t get_client_count() const {
        std::lock_guard<std::mutex> lock(connections_mtx_);
        return connections_.size();
    }

private:
    std::string ip_;
    uint16_t port_;
    WSServer endpoint_;
    std::set<ConnectionHandle, std::owner_less<ConnectionHandle>> connections_;
    mutable std::mutex connections_mtx_;
    std::function<void(const std::string&)> message_handler_;

    // Configure the WebSocket endpoint with handlers.
    void configure_endpoint() {
        // Set error and access logging to be less verbose.
        endpoint_.set_error_channels(websocketpp::log::elevel::info);
        endpoint_.set_access_channels(websocketpp::log::alevel::access_core);
        
        // Clear frame-level logging to reduce noise
        endpoint_.clear_access_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload);

        // Initialize ASIO transport.
        endpoint_.init_asio();

        // Set reuse address option to allow rapid restarts.
        endpoint_.set_reuse_addr(true);

        // Register handlers for connection events.
        endpoint_.set_open_handler([this](ConnectionHandle handle) {
            on_open(handle);
        });

        endpoint_.set_close_handler([this](ConnectionHandle handle) {
            on_close(handle);
        });

        endpoint_.set_message_handler([this](ConnectionHandle handle, WSServer::message_ptr msg) {
            on_message(handle, msg);
        });

        endpoint_.set_fail_handler([this](ConnectionHandle handle) {
            on_fail(handle);
        });
    }

    // Callback when a new connection is established.
    void on_open(ConnectionHandle handle) {
        {
            std::lock_guard<std::mutex> lock(connections_mtx_);
            connections_.insert(handle);
        }
        log_info(LogSource::WSKT, std::string("client connected (") + std::to_string(get_client_count()) + " total)");
    }

    // Callback when a connection is closed.
    void on_close(ConnectionHandle handle) {
        {
            std::lock_guard<std::mutex> lock(connections_mtx_);
            connections_.erase(handle);
        }
        log_info(LogSource::WSKT, std::string("client disconnected (") + std::to_string(get_client_count()) + " total)");
    }

    // Callback when a message is received.
    void on_message(ConnectionHandle, WSServer::message_ptr msg) {
        std::string payload = msg->get_payload();
        log_debug(LogSource::WSKT, std::string("received message: ") + payload);
        
        if (message_handler_) {
            message_handler_(payload);
        }
    }

    // Callback when a connection fails.
    void on_fail(ConnectionHandle) {
        log_warn(LogSource::WSKT, "connection failed");
    }

    // Override loop() from ThreadTask.
    // This runs repeatedly while the server is active (enabled && started).
    void loop() override {
        // Let ASIO process I/O events for a short duration.
        // poll() is non-blocking; poll_one() processes at most one event.
        // We use poll_one() here to keep CPU usage low while staying responsive.
        endpoint_.poll_one();
    }

public:
    // Activate to listen on the endpoint.
    void activate() {
        try {
            // Listen on the configured IP and port
            boost::asio::ip::address bind_addr = boost::asio::ip::make_address(ip_);
            endpoint_.listen(boost::asio::ip::tcp::endpoint(bind_addr, port_));

            // Start accepting connections.
            endpoint_.start_accept();

            log_info(
                LogSource::WSKT,
                std::string("WebSocket server active, listening on ") + ip_ + ":" + std::to_string(port_)
            );
            
        } catch (const std::exception& e) {
            log_error(LogSource::WSKT, std::string("failed to activate: ") + e.what());
            return;
        }

        ThreadTask::activate();
    }

    // Shutdown to close the server gracefully.
    void shutdown() {
        log_info(LogSource::WSKT, "shutting down");

        try {
            // Close all existing connections.
            {
                std::lock_guard<std::mutex> lock(connections_mtx_);
                for (auto& handle : connections_) {
                    try {
                        endpoint_.close(handle, websocketpp::close::status::going_away,
                                      "server shutdown");
                    } catch (...) {
                        // Ignore errors during shutdown
                    }
                }
                connections_.clear();
            }

            // Stop the endpoint (stops accepting new connections and event loop).
            endpoint_.stop_listening();
            endpoint_.stop();
        } catch (const std::exception& e) {
            log_error(LogSource::WSKT, std::string("error during shutdown: ") + e.what());
        }

        ThreadTask::shutdown();
    }
};
