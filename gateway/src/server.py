#!/usr/bin/env python3
"""PC-side PSP-ICE UDP gateway for protocol development."""

import argparse
import socket

from protocol import (
    CHANNEL_CONTROL,
    MSG_HELLO,
    MSG_PING,
    MSG_PONG,
    ProtocolError,
    pack_message,
    unpack_message,
)


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
    print("Waiting for a PC/PSP test client...\n")

    try:
        while True:
            try:
                data, addr = sock.recvfrom(65535)
            except socket.timeout:
                continue

            try:
                message = unpack_message(data)
            except ProtocolError as exc:
                print(f"RX {addr} INVALID: {exc}")
                continue

            print(
                f"RX {addr} channel={message.channel} type={message.msg_type} "
                f"seq={message.sequence} bytes={len(message.payload)}"
            )

            if message.channel != CHANNEL_CONTROL:
                continue

            if message.msg_type == MSG_HELLO:
                sequence += 1
                response = pack_message(
                    CHANNEL_CONTROL,
                    MSG_PONG,
                    sequence,
                    b"PSP-ICE/1",
                )
                sock.sendto(response, addr)
                print(f"TX PONG -> {addr} payload=PSP-ICE/1")

            elif message.msg_type == MSG_PING:
                sequence += 1
                sock.sendto(
                    pack_message(CHANNEL_CONTROL, MSG_PONG, sequence),
                    addr,
                )
                print(f"TX PONG -> {addr}")

            elif message.msg_type == MSG_PONG:
                print(f"PONG from {addr}")
    except KeyboardInterrupt:
        print("\nStopping.")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
