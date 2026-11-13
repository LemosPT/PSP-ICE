#!/usr/bin/env python3
"""Small self-test suite for the PSP-ICE protocol codec."""

import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "gateway" / "src"))

from protocol import (  # noqa: E402
    CHANNEL_CONTROL,
    HEADER,
    MSG_HELLO,
    BUTTON_CROSS,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_SELECT,
    BUTTON_START,
    BUTTON_UP,
    ProtocolError,
    format_buttons,
    pack_message,
    unpack_message,
)


def expect_error(packet: bytes) -> None:
    try:
        unpack_message(packet)
    except ProtocolError:
        return
    raise AssertionError("expected ProtocolError")


def main() -> None:
    payload = b"PC-TEST/1"
    packet = pack_message(CHANNEL_CONTROL, MSG_HELLO, 42, payload)
    message = unpack_message(packet)

    assert len(packet) == HEADER.size + len(payload)
    assert message.channel == CHANNEL_CONTROL
    assert message.msg_type == MSG_HELLO
    assert message.sequence == 42
    assert message.payload == payload

    expected_buttons = {
        BUTTON_SELECT: "SELECT", BUTTON_START: "START", BUTTON_UP: "UP",
        BUTTON_RIGHT: "RIGHT", BUTTON_DOWN: "DOWN", BUTTON_LEFT: "LEFT",
        BUTTON_CROSS: "CROSS",
    }
    assert expected_buttons == {
        1 << 0: "SELECT", 1 << 1: "START", 1 << 2: "UP",
        1 << 3: "RIGHT", 1 << 4: "DOWN", 1 << 5: "LEFT", 1 << 10: "CROSS",
    }
    for mask, name in expected_buttons.items():
        assert format_buttons(mask) == name

    expect_error(b"ICE1")
    expect_error(packet[: HEADER.size - 1])

    bad_magic = bytearray(packet)
    bad_magic[0:4] = b"NOPE"
    expect_error(bytes(bad_magic))

    changed_payload = bytearray(packet)
    changed_payload[-1] ^= 0xFF
    # A payload-byte change is valid because the declared length is unchanged.
    assert unpack_message(bytes(changed_payload)).payload != payload

    truncated_payload = packet[:-1]
    expect_error(truncated_payload)

    print("PSP-ICE protocol tests: PASS")


if __name__ == "__main__":
    main()
