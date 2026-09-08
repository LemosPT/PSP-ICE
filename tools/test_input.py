#!/usr/bin/env python3
"""PC input-state simulator for the PSP-ICE UDP protocol."""

import argparse
import pathlib
import socket
import sys
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "gateway" / "src"))

from protocol import (  # noqa: E402
    CHANNEL_INPUT,
    BUTTON_CROSS,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_START,
    BUTTON_UP,
    MSG_INPUT_STATE,
    format_buttons,
    pack_input_state,
    pack_message,
)

KEYS = {
    "up": BUTTON_UP,
    "down": BUTTON_DOWN,
    "left": BUTTON_LEFT,
    "right": BUTTON_RIGHT,
    "cross": BUTTON_CROSS,
    "start": BUTTON_START,
}


def parse_buttons(text: str) -> int:
    buttons = 0
    if text.strip():
        for name in text.split(","):
            name = name.strip().lower()
            if name not in KEYS:
                raise ValueError(f"unknown button: {name}")
            buttons |= KEYS[name]
    return buttons


def main() -> None:
    parser = argparse.ArgumentParser(description="PSP-ICE PC input simulator")
    parser.add_argument("host", help="gateway IP/hostname")
    parser.add_argument("--port", type=int, default=39000)
    parser.add_argument("--count", type=int, default=20)
    parser.add_argument("--interval", type=float, default=0.05)
    args = parser.parse_args()

    server = (args.host, args.port)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sequence = 0

    print(f"PSP-ICE input test -> {server[0]}:{server[1]}")
    print("Sending a controller-state sequence...\n")

    states = [
        ("NONE", 128, 128),
        ("UP", 128, 0),
        ("DOWN", 128, 255),
        ("LEFT", 0, 128),
        ("RIGHT", 255, 128),
        ("CROSS", 128, 128),
        ("START", 128, 128),
        ("UP,CROSS", 128, 0),
        ("", 128, 128),
    ]

    try:
        for index in range(args.count):
            text, analog_x, analog_y = states[index % len(states)]
            buttons = parse_buttons(text if text != "NONE" else "")
            sequence += 1
            payload = pack_input_state(buttons, analog_x, analog_y)
            packet = pack_message(CHANNEL_INPUT, MSG_INPUT_STATE, sequence, payload)
            sock.sendto(packet, server)
            print(
                f"TX INPUT seq={sequence:04d} buttons={format_buttons(buttons):<16} "
                f"analog=({analog_x:3d},{analog_y:3d})"
            )
            time.sleep(args.interval)
    except KeyboardInterrupt:
        print("\nStopping.")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
