#!/usr/bin/env python3
"""Minimal PSP-ICE development gateway.

This is intentionally a PC-only network prototype. It does not implement
CarPlay yet; it provides the session/control foundation for the PSP link.
"""

import argparse
import socket
import struct
import time

MAGIC = b"ICE1"
VERSION = 1
HEADER = struct.Struct("!4sBBBBIH")
CHANNEL_CONTROL = 0
MSG_HELLO = 1
MSG_PING = 2
MSG_PONG = 3


def pack_message(channel: int, msg_type: int, sequence: int, payload: bytes = b"") -> bytes:
    return HEADER.pack(MAGIC, VERSION, channel, msg_type, 0, sequence, len(payload)) + payload


def main() -> None:
    parser = argparse.ArgumentParser(description="PSP-ICE development gateway")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=39000)
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((args.host, args.port))
    sock.settimeout(1.0)

    sequence = 0
    print(f"PSP-ICE gateway listening on UDP {args.host}:{args.port}")
    print("Waiting for a PSP/test client...")

    while True:
        try:
            data, addr = sock.recvfrom(2048)
        except socket.timeout:
            continue
        except KeyboardInterrupt:
            print("\nStopping.")
            break

        if len(data) < HEADER.size:
            continue

        magic, version, channel, msg_type, _flags, rx_sequence, length = HEADER.unpack_from(data)
        payload = data[HEADER.size:]

        if magic != MAGIC or version != VERSION or length != len(payload):
            print(f"Ignoring malformed packet from {addr}")
            continue

        print(f"RX {addr} channel={channel} type={msg_type} seq={rx_sequence} bytes={length}")

        if channel == CHANNEL_CONTROL and msg_type == MSG_HELLO:
            sequence += 1
            sock.sendto(pack_message(CHANNEL_CONTROL, MSG_PONG, sequence, b"PSP-ICE/1"), addr)
            print(f"TX PONG → {addr}")

        elif channel == CHANNEL_CONTROL and msg_type == MSG_PING:
            sequence += 1
            sock.sendto(pack_message(CHANNEL_CONTROL, MSG_PONG, sequence), addr)


if __name__ == "__main__":
    main()
