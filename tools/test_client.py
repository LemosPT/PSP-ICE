#!/usr/bin/env python3
"""Interactive PC client for testing the PSP-ICE UDP protocol."""

import argparse
import pathlib
import socket
import sys
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "gateway" / "src"))

from protocol import CHANNEL_CONTROL, MSG_HELLO, MSG_PING, pack_message, unpack_message


def send_and_wait(sock: socket.socket, server: tuple[str, int], packet: bytes, timeout: float):
    sock.sendto(packet, server)
    sock.settimeout(timeout)
    while True:
        data, addr = sock.recvfrom(65535)
        try:
            message = unpack_message(data)
        except ValueError as exc:
            print(f"RX {addr} INVALID: {exc}")
            continue
        return message, addr


def main() -> None:
    parser = argparse.ArgumentParser(description="PSP-ICE PC protocol test client")
    parser.add_argument("host", help="gateway IP/hostname")
    parser.add_argument("--port", type=int, default=39000)
    parser.add_argument("--count", type=int, default=5, help="number of pings")
    parser.add_argument("--interval", type=float, default=1.0)
    parser.add_argument("--timeout", type=float, default=2.0)
    args = parser.parse_args()

    server = (args.host, args.port)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sequence = 0

    print(f"PSP-ICE client -> {server[0]}:{server[1]}")

    try:
        sequence += 1
        hello = pack_message(CHANNEL_CONTROL, MSG_HELLO, sequence, b"PC-TEST/1")
        started = time.monotonic()
        response, _ = send_and_wait(sock, server, hello, args.timeout)
        elapsed_ms = (time.monotonic() - started) * 1000
        payload = response.payload.decode("utf-8", errors="replace")
        print(
            f"HELLO -> PONG seq={response.sequence} payload={payload!r} "
            f"RTT={elapsed_ms:.2f} ms"
        )

        for index in range(1, args.count + 1):
            sequence += 1
            ping = pack_message(CHANNEL_CONTROL, MSG_PING, sequence)
            started = time.monotonic()
            try:
                response, _ = send_and_wait(sock, server, ping, args.timeout)
            except socket.timeout:
                print(f"PING {index}: TIMEOUT")
            else:
                elapsed_ms = (time.monotonic() - started) * 1000
                print(f"PING {index}: PONG seq={response.sequence} RTT={elapsed_ms:.2f} ms")
            if index != args.count:
                time.sleep(args.interval)

    except socket.timeout:
        print("HELLO: TIMEOUT")
        raise SystemExit(1)
    finally:
        sock.close()


if __name__ == "__main__":
    main()
