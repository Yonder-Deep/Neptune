#pragma once

/**
 * @file ws_server.hpp
 * @brief WebSocket drop-in replacement for tcp_server.hpp
 *
 * Identical public API — swap the #include and the class name in
 * motor_server.cpp and browsers can connect directly via WebSocket.
 *
 * No new library dependencies. Uses POSIX sockets and an inline SHA-1
 * + Base64 implementation for the WebSocket opening handshake (RFC 6455).
 *
 * Limitations (acceptable for this use-case):
 *   - One client at a time (same as TcpServer)
 *   - Text frames only; binary/continuation frames are silently skipped
 *   - Messages must fit in a single frame (fine for short JSON commands)
 *   - No TLS — suitable for a closed RF link, not the public internet
 *
 * For HolyBro Telemetry Radio:
 *   on Pi:   sudo pppd /dev/ttyUSB0 57600 lock local noauth 192.168.100.11:192.168.100.10
 *   on base: sudo pppd /dev/ttyUSB0 57600 lock local noauth nodetach 192.168.100.10:192.168.100.11
 *
 *   Browser connects to: ws://192.168.100.11:<port>
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Internal helpers — SHA-1 (RFC 3174) and Base64 for the WS handshake.
// Only called once per connection so performance is irrelevant.
// ---------------------------------------------------------------------------
namespace ws_detail {

static void sha1(const uint8_t *msg, size_t len, uint8_t digest[20]) {
  uint32_t H[5] = {0x67452301u, 0xEFCDAB89u, 0x98BADCFEu,
                   0x10325476u, 0xC3D2E1F0u};

  // Pad message to a multiple of 64 bytes (RFC 3174 §4)
  uint64_t bit_len = static_cast<uint64_t>(len) * 8;
  std::vector<uint8_t> p(msg, msg + len);
  p.push_back(0x80);
  while (p.size() % 64 != 56)
    p.push_back(0x00);
  for (int i = 7; i >= 0; --i)
    p.push_back(static_cast<uint8_t>(bit_len >> (i * 8)));

  auto rotl = [](uint32_t v, int n) -> uint32_t {
    return (v << n) | (v >> (32 - n));
  };

  for (size_t off = 0; off < p.size(); off += 64) {
    uint32_t W[80];
    for (int j = 0; j < 16; ++j)
      W[j] = ((uint32_t)p[off + j * 4] << 24) |
              ((uint32_t)p[off + j * 4 + 1] << 16) |
              ((uint32_t)p[off + j * 4 + 2] << 8) |
              (uint32_t)p[off + j * 4 + 3];
    for (int j = 16; j < 80; ++j)
      W[j] = rotl(W[j - 3] ^ W[j - 8] ^ W[j - 14] ^ W[j - 16], 1);

    uint32_t a = H[0], b = H[1], c = H[2], d = H[3], e = H[4];
    for (int j = 0; j < 80; ++j) {
      uint32_t f, k;
      if (j < 20)      { f = (b & c) | (~b & d); k = 0x5A827999u; }
      else if (j < 40) { f = b ^ c ^ d;           k = 0x6ED9EBA1u; }
      else if (j < 60) { f = (b&c)|(b&d)|(c&d);  k = 0x8F1BBCDCu; }
      else             { f = b ^ c ^ d;           k = 0xCA62C1D6u; }
      uint32_t t = rotl(a, 5) + f + e + k + W[j];
      e = d; d = c; c = rotl(b, 30); b = a; a = t;
    }
    H[0] += a; H[1] += b; H[2] += c; H[3] += d; H[4] += e;
  }

  for (int i = 0; i < 5; ++i) {
    digest[i * 4]     = (H[i] >> 24) & 0xFF;
    digest[i * 4 + 1] = (H[i] >> 16) & 0xFF;
    digest[i * 4 + 2] = (H[i] >>  8) & 0xFF;
    digest[i * 4 + 3] =  H[i]        & 0xFF;
  }
}

static std::string base64_encode(const uint8_t *data, size_t len) {
  static const char *T =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  out.reserve(((len + 2) / 3) * 4);
  for (size_t i = 0; i < len; i += 3) {
    uint32_t v = static_cast<uint32_t>(data[i]) << 16;
    if (i + 1 < len) v |= static_cast<uint32_t>(data[i + 1]) << 8;
    if (i + 2 < len) v |= data[i + 2];
    out += T[(v >> 18) & 63];
    out += T[(v >> 12) & 63];
    out += (i + 1 < len) ? T[(v >> 6) & 63] : '=';
    out += (i + 2 < len) ? T[v & 63]        : '=';
  }
  return out;
}

// Computes the Sec-WebSocket-Accept value from the client's key (RFC 6455 §4.2.2)
static std::string make_accept(const std::string &key) {
  const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  std::string combined    = key + magic;
  uint8_t digest[20];
  sha1(reinterpret_cast<const uint8_t *>(combined.data()), combined.size(),
       digest);
  return base64_encode(digest, 20);
}

} // namespace ws_detail

// ---------------------------------------------------------------------------
// WsServer — same public API as TcpServer
// ---------------------------------------------------------------------------
class WsServer {
public:
  explicit WsServer(int port) : port_(port), server_fd_(-1), client_fd_(-1) {}

  ~WsServer() { close_all(); }

  /**
   * Bind and start listening. Call once before the main loop.
   */
  void start() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0)
      throw std::runtime_error("socket() failed: " + err());

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port_));

    if (bind(server_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
      throw std::runtime_error("bind() failed: " + err());

    if (listen(server_fd_, 1) < 0)
      throw std::runtime_error("listen() failed: " + err());

    std::cout << "[WS] Listening on port " << port_ << std::endl;
  }

  /**
   * Block until a WebSocket client connects and completes the opening handshake.
   *
   * Also handles plain HTTP GET requests gracefully — returns a small JSON
   * status body so curl/health-checks work without a WebSocket client.
   * If a non-WS connection is received the method recurses to wait for a
   * real WebSocket connection.
   */
  void accept_client() {
    if (client_fd_ >= 0) {
      close(client_fd_);
      client_fd_ = -1;
    }

    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    std::cout << "[WS] Waiting for connection..." << std::endl;

    client_fd_ =
        accept(server_fd_, reinterpret_cast<sockaddr *>(&addr), &len);
    if (client_fd_ < 0)
      throw std::runtime_error("accept() failed: " + err());

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    std::cout << "[WS] TCP connection from " << ip << std::endl;

    if (!do_handshake()) {
      // Plain HTTP or bad handshake — close and wait for a real WS client.
      close(client_fd_);
      client_fd_ = -1;
      accept_client();
      return;
    }

    std::cout << "[WS] WebSocket handshake complete." << std::endl;
  }

  /**
   * Receive one WebSocket text message.
   *
   * Blocks until a complete frame arrives. Returns false if the client
   * disconnects or sends a Close frame. Ping frames are answered
   * automatically with a Pong; all other non-text frames are skipped.
   */
  bool recv_line(std::string &out) {
    out.clear();
    while (true) {
      // --- 2-byte header ---
      uint8_t hdr[2];
      if (!recv_exact(hdr, 2))
        return false;

      bool    fin    = (hdr[0] & 0x80) != 0;
      uint8_t opcode =  hdr[0] & 0x0F;
      bool    masked = (hdr[1] & 0x80) != 0;
      uint64_t plen  =  hdr[1] & 0x7F;

      // --- Extended payload length ---
      if (plen == 126) {
        uint8_t ext[2];
        if (!recv_exact(ext, 2))
          return false;
        plen = ((uint64_t)ext[0] << 8) | ext[1];
      }
      else if (plen == 127) {
        uint8_t ext[8];
        if (!recv_exact(ext, 8))
          return false;
        plen = 0;
        for (int i = 0; i < 8; ++i)
          plen = (plen << 8) | ext[i];
      }

      // --- Masking key (clients MUST mask; servers MUST NOT) ---
      uint8_t mask[4] = {};
      if (masked && !recv_exact(mask, 4))
        return false;

      // --- Payload ---
      std::vector<uint8_t> payload(static_cast<size_t>(plen));
      if (plen > 0 && !recv_exact(payload.data(), static_cast<size_t>(plen)))
        return false;

      if (masked)
        for (size_t i = 0; i < payload.size(); ++i)
          payload[i] ^= mask[i % 4];

      // --- Dispatch by opcode ---
      switch (opcode) {
        case 0x1: // Text
        case 0x2: // Binary — treat as text for our JSON protocol
          out.assign(payload.begin(), payload.end());
          return true;

        case 0x8: // Close
          send_close();
          std::cout << "[WS] Client sent Close frame." << std::endl;
          return false;

        case 0x9: // Ping — reply with Pong
          send_frame(0xA, payload);
          continue;

        default: // Continuation, Pong, reserved — skip
          continue;
      }
    }
  }

  /**
   * Send msg as a WebSocket text frame (opcode 0x1, FIN set, unmasked).
   * Returns false if the send failed.
   */
  bool send_line(const std::string &msg) {
    std::vector<uint8_t> payload(msg.begin(), msg.end());
    return send_frame(0x1, payload);
  }

  bool has_client() const { return client_fd_ >= 0; }

private:
  int port_;
  int server_fd_;
  int client_fd_;

  // -------------------------------------------------------------------------
  // WebSocket opening handshake (RFC 6455 §4)
  // -------------------------------------------------------------------------
  bool do_handshake() {
    // Read the HTTP request, stop at the blank line (\r\n\r\n).
    std::string request;
    char ch;
    while (true) {
      ssize_t n = recv(client_fd_, &ch, 1, 0);
      if (n <= 0)
        return false;
      request += ch;
      if (request.size() >= 4 &&
          request.substr(request.size() - 4) == "\r\n\r\n")
        break;
      // Guard against a malicious or accidental huge request.
      if (request.size() > 8192)
        return false;
    }

    // Not a WebSocket upgrade? Send a plain HTTP response so curl works.
    bool is_ws = request.find("Upgrade: websocket") != std::string::npos ||
                 request.find("Upgrade: Websocket") != std::string::npos;
    if (!is_ws) {
      std::string body = "{\"ok\":true,\"msg\":\"motor_server running — "
                         "connect via WebSocket\"}";
      std::string resp =
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: application/json\r\n"
          "Content-Length: " + std::to_string(body.size()) + "\r\n"
          "Connection: close\r\n"
          "\r\n" + body;
      send(client_fd_, resp.data(), resp.size(), MSG_NOSIGNAL);
      return false;
    }

    // Extract Sec-WebSocket-Key
    const std::string key_header = "Sec-WebSocket-Key: ";
    size_t pos = request.find(key_header);
    if (pos == std::string::npos)
      return false;
    pos += key_header.size();
    size_t end = request.find("\r\n", pos);
    if (end == std::string::npos)
      return false;
    std::string key = request.substr(pos, end - pos);

    // Send 101 Switching Protocols
    std::string accept   = ws_detail::make_accept(key);
    std::string response =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: " + accept + "\r\n"
        "\r\n";

    ssize_t sent = send(client_fd_, response.data(), response.size(),
                        MSG_NOSIGNAL);
    return sent == static_cast<ssize_t>(response.size());
  }

  // -------------------------------------------------------------------------
  // Frame helpers
  // -------------------------------------------------------------------------

  // Build and send a single WebSocket frame (server→client, always unmasked).
  bool send_frame(uint8_t opcode, const std::vector<uint8_t> &payload) {
    std::vector<uint8_t> frame;
    frame.push_back(0x80 | opcode); // FIN=1 + opcode

    const size_t plen = payload.size();
    if (plen < 126) {
      frame.push_back(static_cast<uint8_t>(plen));
    }
    else if (plen <= 0xFFFF) {
      frame.push_back(126);
      frame.push_back((plen >> 8) & 0xFF);
      frame.push_back(plen & 0xFF);
    }
    else {
      frame.push_back(127);
      for (int i = 7; i >= 0; --i)
        frame.push_back((plen >> (i * 8)) & 0xFF);
    }

    frame.insert(frame.end(), payload.begin(), payload.end());
    ssize_t n = send(client_fd_, frame.data(), frame.size(), MSG_NOSIGNAL);
    return n == static_cast<ssize_t>(frame.size());
  }

  void send_close() {
    // Minimal Close frame: FIN + opcode 0x8, no payload, no mask.
    uint8_t frame[2] = {0x88, 0x00};
    send(client_fd_, frame, 2, MSG_NOSIGNAL);
  }

  // Read exactly n bytes, retrying on short reads.
  bool recv_exact(uint8_t *buf, size_t n) {
    size_t got = 0;
    while (got < n) {
      ssize_t r = recv(client_fd_, buf + got, n - got, 0);
      if (r <= 0) {
        std::cout << "[WS] Client disconnected." << std::endl;
        return false;
      }
      got += r;
    }
    return true;
  }

  void close_all() {
    if (client_fd_ >= 0) { close(client_fd_); client_fd_ = -1; }
    if (server_fd_ >= 0) { close(server_fd_); server_fd_ = -1; }
  }

  static std::string err() { return std::string(strerror(errno)); }
};
