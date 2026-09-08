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
    ProtocolError,
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

    expect_error(b"ICE1")
    expect_error(packet[: HEADER.size - 1])

    bad_magic = bytearray(packet)
    bad_magic[0:4] = b"NOPE"
    expect_error(bytes(bad_magic))

    bad_length = bytearray(packet)
    bad_length[-1] ^= 0xFF
    # This changes payload bytes, not its length, so it remains valid.
    assert unpack_message(bytes(bad_length)).payload != payload

    truncated_payload = packet[:-1]
    expect_error(truncated_payload)

    print("PSP-ICE protocol tests: PASS")


if __name__ == "__main__":
    main()
