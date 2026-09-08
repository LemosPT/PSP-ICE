#!/usr/bin/env python3
"""PC-side PSP-ICE UDP gateway for protocol development."""

import argparse
import socket

from protocol import (
    CHANNEL_CONTROL,
    CHANNEL_INPUT,
    MSG_HELLO,
    MSG_INPUT_STATE,
    MSG_PING,
    MSG_PONG,
    ProtocolError,
    format_buttons,
    pack_message,
    unpack_input_state,
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

            if message.channel == CHANNEL_CONTROL:
                if message.msg_type == MSG_HELLO:
                    sequence += 1
                    response = pack_message(
                        CHANNEL_CONTROL, MSG_PONG, sequence, b"PSP-ICE/1"
                    )
                    sock.sendto(response, addr)
                    print(f"TX PONG -> {addr} payload=PSP-ICE/1")

                elif message.msg_type == MSG_PING:
                    sequence += 1
                    sock.sendto(
                        pack_message(CHANNEL_CONTROL, MSG_PONG, sequence), addr
                    )
                    print(f"TX PONG -> {addr}")

                elif message.msg_type == MSG_PONG:
                    print(f"PONG from {addr}")

            elif message.channel == CHANNEL_INPUT and message.msg_type == MSG_INPUT_STATE:
                try:
                    buttons, analog_x, analog_y = unpack_input_state(message.payload)
                except ProtocolError as exc:
                    print(f"INPUT ERROR: {exc}")
                    continue
                print(
                    f"INPUT seq={message.sequence} buttons={format_buttons(buttons)} "
                    f"analog=({analog_x},{analog_y})"
                )
    except KeyboardInterrupt:
        print("\nStopping.")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
