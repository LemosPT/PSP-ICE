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
