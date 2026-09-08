"""PSP-ICE v1 binary protocol codec."""

from dataclasses import dataclass
import struct

MAGIC = b"ICE1"
VERSION = 1

# magic, version, channel, type, flags, sequence, payload_length
HEADER = struct.Struct("!4sBBBBIH")
MAX_PAYLOAD = 60_000

CHANNEL_CONTROL = 0
CHANNEL_VIDEO = 1
CHANNEL_INPUT = 2
CHANNEL_AUDIO = 3

MSG_HELLO = 1
MSG_PING = 2
MSG_PONG = 3
MSG_INPUT_STATE = 10
MSG_VIDEO_FRAME = 20

# INPUT_STATE payload:
# buttons: uint16 bitmask, analog_x: uint8, analog_y: uint8
INPUT_STATE = struct.Struct("!HBB")

# PSP button bitmasks. Keep these stable for the PSP implementation.
BUTTON_UP = 1 << 0
BUTTON_DOWN = 1 << 1
BUTTON_LEFT = 1 << 2
BUTTON_RIGHT = 1 << 3
BUTTON_TRIANGLE = 1 << 4
BUTTON_CIRCLE = 1 << 5
BUTTON_CROSS = 1 << 6
BUTTON_SQUARE = 1 << 7
BUTTON_LTRIGGER = 1 << 8
BUTTON_RTRIGGER = 1 << 9
BUTTON_START = 1 << 10
BUTTON_SELECT = 1 << 11
BUTTON_HOME = 1 << 12
BUTTON_HOLD = 1 << 13

BUTTON_NAMES = {
    BUTTON_UP: "UP",
    BUTTON_DOWN: "DOWN",
    BUTTON_LEFT: "LEFT",
    BUTTON_RIGHT: "RIGHT",
    BUTTON_TRIANGLE: "TRIANGLE",
    BUTTON_CIRCLE: "CIRCLE",
    BUTTON_CROSS: "CROSS",
    BUTTON_SQUARE: "SQUARE",
    BUTTON_LTRIGGER: "L",
    BUTTON_RTRIGGER: "R",
    BUTTON_START: "START",
    BUTTON_SELECT: "SELECT",
    BUTTON_HOME: "HOME",
    BUTTON_HOLD: "HOLD",
}


class ProtocolError(ValueError):
    """Raised when a PSP-ICE packet is malformed or unsupported."""


@dataclass(frozen=True)
class Message:
    channel: int
    msg_type: int
    sequence: int
    payload: bytes = b""
    flags: int = 0


def pack_message(
    channel: int,
    msg_type: int,
    sequence: int,
    payload: bytes = b"",
    flags: int = 0,
) -> bytes:
    if not 0 <= channel <= 255:
        raise ProtocolError("channel must fit in one byte")
    if not 0 <= msg_type <= 255:
        raise ProtocolError("message type must fit in one byte")
    if not 0 <= flags <= 255:
        raise ProtocolError("flags must fit in one byte")
    if not 0 <= sequence <= 0xFFFFFFFF:
        raise ProtocolError("sequence must fit in uint32")
    if len(payload) > MAX_PAYLOAD:
        raise ProtocolError(f"payload exceeds {MAX_PAYLOAD} bytes")

    return HEADER.pack(
        MAGIC, VERSION, channel, msg_type, flags, sequence, len(payload)
    ) + payload


def pack_input_state(buttons: int, analog_x: int = 128, analog_y: int = 128) -> bytes:
    """Encode the current PSP controller state into an INPUT_STATE payload."""
    if not 0 <= buttons <= 0xFFFF:
        raise ProtocolError("buttons must fit in uint16")
    if not 0 <= analog_x <= 255 or not 0 <= analog_y <= 255:
        raise ProtocolError("analog values must fit in uint8")
    return INPUT_STATE.pack(buttons, analog_x, analog_y)


def unpack_input_state(payload: bytes) -> tuple[int, int, int]:
    """Decode an INPUT_STATE payload as (buttons, analog_x, analog_y)."""
    if len(payload) != INPUT_STATE.size:
        raise ProtocolError("INPUT_STATE payload must be exactly 4 bytes")
    return INPUT_STATE.unpack(payload)


def format_buttons(buttons: int) -> str:
    """Return a human-readable button list for diagnostics."""
    names = [name for mask, name in BUTTON_NAMES.items() if buttons & mask]
    return "+".join(names) if names else "NONE"


def unpack_message(data: bytes) -> Message:
    if len(data) < HEADER.size:
        raise ProtocolError("packet shorter than header")

    magic, version, channel, msg_type, flags, sequence, length = HEADER.unpack_from(data)
    payload = data[HEADER.size:]

    if magic != MAGIC:
        raise ProtocolError("bad magic")
    if version != VERSION:
        raise ProtocolError(f"unsupported protocol version {version}")
    if length != len(payload):
        raise ProtocolError("payload length does not match header")
    if length > MAX_PAYLOAD:
        raise ProtocolError("payload exceeds maximum")

    return Message(channel, msg_type, sequence, payload, flags)
