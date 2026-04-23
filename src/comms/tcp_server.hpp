#pragma once

/**
 * @file tcp_server.hpp
 * @brief Minimal single-client TCP server using POSIX sockets.
 *
 * Accepts one connection at a time. Each call to recv_line() blocks until a
 * newline-terminated message arrives. The server automatically re-accepts after
 * a client disconnects.
 *
 * No external dependencies — POSIX only.
 * 
 * For HolyBro Telemetry Radio:
     ensure that radios are connected to Pi and Base Station
     on Pi: sudo pppd /dev/ttyUSB0 57600 lock local noauth 192.168.100.10:192.168.100.11
     on base: sudo pppd /dev/ttyUSB0 57600 lock local noauth nodetach 192.168.100.10:192.168.100.11
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

class TcpServer {
public:
    explicit TcpServer(int port) : port_(port), server_fd_(-1), client_fd_(-1) {}

    ~TcpServer() { close_all(); }

    /**
     * Bind and start listening. Call once before the main loop.
     */
    void start() {
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) throw std::runtime_error("socket() failed: " + err());

        // Allow rapid restart without "Address already in use"
        int opt = 1;
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(static_cast<uint16_t>(port_));

        if (bind(server_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
            throw std::runtime_error("bind() failed: " + err());

        if (listen(server_fd_, 1) < 0)
            throw std::runtime_error("listen() failed: " + err());

        std::cout << "[TCP] Listening on port " << port_ << std::endl;
    }

    /**
     * Block until a client connects. Closes any existing client first.
     */
    void accept_client() {
        if (client_fd_ >= 0) {
            close(client_fd_);
            client_fd_ = -1;
        }
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        std::cout << "[TCP] Waiting for connection..." << std::endl;
        client_fd_ = accept(server_fd_, reinterpret_cast<sockaddr*>(&client_addr), &len);
        if (client_fd_ < 0) throw std::runtime_error("accept() failed: " + err());

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        std::cout << "[TCP] Client connected from " << ip << std::endl;
    }

    /**
     * Read one newline-terminated line from the current client.
     * Returns false if the client disconnected (caller should call accept_client again).
     */
    bool recv_line(std::string& out) {
        out.clear();
        char ch;
        while (true) {
            ssize_t n = recv(client_fd_, &ch, 1, 0);
            if (n <= 0) {
                std::cout << "[TCP] Client disconnected." << std::endl;
                return false;
            }
            if (ch == '\n') return true;
            if (ch != '\r') out += ch;
        }
    }

    /**
     * Send a newline-terminated string to the current client.
     * Returns false if the send failed.
     */
    bool send_line(const std::string& msg) {
        std::string out = msg + "\n";
        ssize_t n = send(client_fd_, out.c_str(), out.size(), MSG_NOSIGNAL);
        return n == static_cast<ssize_t>(out.size());
    }

    bool has_client() const { return client_fd_ >= 0; }

private:
    int port_;
    int server_fd_;
    int client_fd_;

    void close_all() {
        if (client_fd_ >= 0) { close(client_fd_); client_fd_ = -1; }
        if (server_fd_ >= 0) { close(server_fd_); server_fd_ = -1; }
    }

    static std::string err() { return std::string(strerror(errno)); }
};