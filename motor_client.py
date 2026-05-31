#!/usr/bin/env python3
"""
motor_client.py  —  Base station client for motor_server

Connects to the Pi's motor_server over TCP (WiFi HaLow, Ethernet, USB-gadget,
or localhost) and provides an interactive command line to control motors.

For HolyBro Telemetry Radio:
    ensure that radios are connected to Pi and Base Station
    on Pi: sudo pppd /dev/ttyUSB0 57600 lock local noauth 192.168.100.11:192.168.100.10
    on base: sudo pppd /dev/ttyUSB0 57600 lock local noauth nodetach 192.168.100.10:192.168.100.11

Usage:
    python3 motor_client.py                      # localhost:9000 (both sides local)
    python3 motor_client.py 192.168.100.11       # Pi via static Ethernet/HaLow
    python3 motor_client.py 192.168.100.11 9000  # explicit port

Commands (once connected):
    motors <f> <t> <fr> <bk>   Set forward, turn, front, back [-1.0 to 1.0]
    pwm <index> <microseconds>  Set one motor by raw PWM (1100–1900 µs)
    stop                        Zero all motors
    status                      Query current normalized speed for all motors
    quit                        Shut down the server and exit
    help                        Show this message
    Ctrl+C or exit              Disconnect and exit client only
"""

import json
import socket
import sys
import readline  # noqa: F401 — gives line editing / history for free


DEFAULT_HOST = "localhost"
DEFAULT_PORT = 9000


def send_cmd(sock: socket.socket, obj: dict) -> dict | None:
    """Send a JSON command and return the parsed response."""
    try:
        msg = json.dumps(obj) + "\n"
        sock.sendall(msg.encode())
        # Read until newline
        buf = b""
        while not buf.endswith(b"\n"):
            chunk = sock.recv(4096)
            if not chunk:
                return None
            buf += chunk
        return json.loads(buf.strip())
    except (OSError, json.JSONDecodeError) as e:
        print(f"[ERR] {e}")
        return None


def print_response(resp: dict | None):
    if resp is None:
        print("[ERR] No response (disconnected?)")
        return
    ok = resp.get("ok", False)
    prefix = "[ OK]" if ok else "[ERR]"

    # Status response has speeds array (normalized -1.0 to 1.0)
    if "speeds" in resp:
        speeds = resp["speeds"]
        labels = ["forward", "turn  ", "front ", "back  "]
        print(f"{prefix} Motor speeds (normalized):")
        for i, (label, speed) in enumerate(zip(labels, speeds)):
            bar_len = int(abs(speed) * 20)
            direction = "►" if speed >= 0 else "◄"
            bar = direction * bar_len
            print(f"       [{i}] {label}: {speed:+.3f}  {bar}")
    else:
        print(f"{prefix} {resp.get('msg', resp)}")


def print_help():
    print("""
Commands:
  motors <fwd> <turn> <front> <back>   Normalized values, -1.0 to 1.0
                                        fwd   = forward/reverse thruster
                                        turn  = yaw thruster
                                        front = front vertical thruster
                                        back  = back vertical thruster

  pwm <index> <microseconds>           Raw PWM for one motor (1100–1900 µs)
                                        1500 = neutral/stop
                                        1900 = full forward
                                        1100 = full reverse

  stop                                 Stop all motors
  status                               Show current normalized speed for all motors
  quit                                 Shut down the server then exit
  help                                 Show this message
  exit / Ctrl+C                        Disconnect client (server keeps running)
""")


def main():
    host = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_HOST
    port = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_PORT

    print(f"[CLIENT] Connecting to {host}:{port} ...")
    try:
        sock = socket.create_connection((host, port), timeout=5)
        sock.settimeout(None)  # blocking after connect
    except OSError as e:
        print(f"[ERR] Could not connect: {e}")
        sys.exit(1)

    # Read the greeting
    buf = b""
    while not buf.endswith(b"\n"):
        buf += sock.recv(4096)
    greeting = json.loads(buf.strip())
    print_response(greeting)
    print_help()

    try:
        while True:
            try:
                raw = input("motor> ").strip()
            except EOFError:
                break

            if not raw:
                continue

            parts = raw.split()
            cmd = parts[0].lower()

            if cmd in ("exit", "disconnect"):
                print("[CLIENT] Disconnecting (server still running).")
                break

            elif cmd == "help":
                print_help()

            elif cmd == "stop":
                resp = send_cmd(sock, {"command": "stop"})
                print_response(resp)

            elif cmd == "status":
                resp = send_cmd(sock, {"command": "status"})
                print_response(resp)

            elif cmd == "quit":
                resp = send_cmd(sock, {"command": "quit"})
                print_response(resp)
                break

            elif cmd == "motors":
                if len(parts) != 5:
                    print("[ERR] Usage: motors <fwd> <turn> <front> <back>")
                    continue
                try:
                    vals = [float(p) for p in parts[1:]]
                except ValueError:
                    print("[ERR] Values must be floats in [-1.0, 1.0]")
                    continue
                if any(abs(v) > 1.0 for v in vals):
                    print("[WARN] Values outside [-1, 1] will be clamped by the server.")
                resp = send_cmd(sock, {"command": "motors", "values": vals})
                print_response(resp)

            elif cmd == "pwm":
                if len(parts) != 3:
                    print("[ERR] Usage: pwm <motor_index> <microseconds>")
                    continue
                try:
                    idx = int(parts[1])
                    val = int(parts[2])
                except ValueError:
                    print("[ERR] Index and value must be integers.")
                    continue
                resp = send_cmd(sock, {"command": "pwm", "motor": idx, "value": val})
                print_response(resp)

            else:
                print(f"[ERR] Unknown command '{cmd}'. Type 'help' for options.")

    except KeyboardInterrupt:
        print("\n[CLIENT] Interrupted.")
    finally:
        sock.close()
        print("[CLIENT] Connection closed.")


if __name__ == "__main__":
    main()
