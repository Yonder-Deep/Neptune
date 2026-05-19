#!/usr/bin/env python3
"""
websocket_client.py  —  Base station client for motor_server (WebSocket)

Connects to the Pi's motor_server over WebSocket and provides an interactive
command line to control motors.

Usage:
    python3 websocket_client.py                      # ws://localhost:9000
    python3 websocket_client.py 192.168.100.11       # ws://192.168.100.11:9000
    python3 websocket_client.py 192.168.100.11 9000  # explicit port
    python3 websocket_client.py ws://192.168.100.11:9000  # full URL

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
import readline  # noqa: F401 — gives line editing / history for free
import sys
from typing import Any

try:
    import websocket
except ImportError:
    print("[ERR] Missing dependency: websocket-client")
    print("      Install with: pip3 install websocket-client")
    sys.exit(1)


DEFAULT_HOST = "localhost"
DEFAULT_PORT = 9000


def build_ws_url(host: str, port: int) -> str:
    if host.startswith("ws://") or host.startswith("wss://"):
        return host
    return f"ws://{host}:{port}"


def recv_json(ws: "websocket.WebSocket") -> dict[str, Any] | None:
    try:
        msg = ws.recv()
        if msg is None:
            return None
        if isinstance(msg, (bytes, bytearray)):
            msg = msg.decode()
        return json.loads(msg)
    except (websocket.WebSocketConnectionClosedException, OSError) as e:
        print(f"[ERR] {e}")
        return None
    except json.JSONDecodeError as e:
        print(f"[ERR] Bad JSON from server: {e}")
        return None


def send_cmd(ws: "websocket.WebSocket", obj: dict[str, Any]) -> dict[str, Any] | None:
    try:
        ws.send(json.dumps(obj))
        return recv_json(ws)
    except (websocket.WebSocketConnectionClosedException, OSError) as e:
        print(f"[ERR] {e}")
        return None


def print_response(resp: dict | None):
    if resp is None:
        print("[ERR] No response (disconnected?)")
        return
    ok = resp.get("ok", False)
    prefix = "[ OK]" if ok else "[ERR]"

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
    print(
        """
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
"""
    )


def main():
    host = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_HOST
    port = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_PORT
    url = build_ws_url(host, port)

    print(f"[CLIENT] Connecting to {url} ...")
    try:
        ws = websocket.create_connection(url, timeout=5)
        ws.settimeout(None)  # blocking after connect
    except OSError as e:
        print(f"[ERR] Could not connect: {e}")
        sys.exit(1)

    # Read the greeting
    greeting = recv_json(ws)
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
                resp = send_cmd(ws, {"command": "stop"})
                print_response(resp)

            elif cmd == "status":
                resp = send_cmd(ws, {"command": "status"})
                print_response(resp)

            elif cmd == "quit":
                resp = send_cmd(ws, {"command": "quit"})
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
                    print(
                        "[WARN] Values outside [-1, 1] will be clamped by the server."
                    )
                resp = send_cmd(ws, {"command": "motors", "values": vals})
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
                resp = send_cmd(ws, {"command": "pwm", "motor": idx, "value": val})
                print_response(resp)

            else:
                print(f"[ERR] Unknown command '{cmd}'. Type 'help' for options.")

    except KeyboardInterrupt:
        print("\n[CLIENT] Interrupted.")
    finally:
        ws.close()
        print("[CLIENT] Connection closed.")


if __name__ == "__main__":
    main()
