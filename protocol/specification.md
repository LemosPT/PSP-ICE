# PSP-ICE Transport Specification

## Goals

- Low latency
- Small packets suitable for PSP Wi-Fi
- Video may tolerate packet loss
- Input events must not be silently reordered
- Keep the protocol simple enough to implement in PSP homebrew
- Allow the video transport to evolve from raw frames to compressed video later

## Transport

PSP-ICE uses UDP for the prototype transport.

Default gateway port:

```text
39000/UDP
```

UDP is intentionally used because video and input are latency-sensitive. The protocol does not require reliable delivery of every packet. Higher-level logic is responsible for detecting missing or obsolete data.

## Logical channels

| Channel | Value | Direction | Purpose |
|---|---:|---|---|
| CONTROL | 0 | Bidirectional | Session setup, capabilities, keepalive |
| VIDEO | 1 | Gateway → PSP | Display frames |
| INPUT | 2 | PSP → Gateway | Buttons and analog input |
| AUDIO | 3 | Gateway → PSP | Reserved for future digital audio transport |

## Message header

Every message begins with a fixed 14-byte header in network byte order (big endian):

| Field | Size | Description |
|---|---:|---|
| Magic | 4 bytes | ASCII `ICE1` |
| Version | 1 byte | Protocol version, currently `1` |
| Channel | 1 byte | Logical channel |
| Type | 1 byte | Message type |
| Flags | 1 byte | Message-specific flags; `0` unless defined otherwise |
| Sequence | 4 bytes | Monotonically increasing message sequence number |
| Payload length | 2 bytes | Number of payload bytes following the header |

Total header size: **14 bytes**.

Maximum payload length is **60,000 bytes** for the current implementation.

## Message types

| Type | Value | Channel | Purpose |
|---|---:|---|---|
| HELLO | 1 | CONTROL | Announce a PSP-ICE endpoint |
| PING | 2 | CONTROL | Keepalive / connectivity test |
| PONG | 3 | CONTROL | Response to HELLO/PING |
| INPUT_STATE | 10 | INPUT | Current PSP button and analog state |
| VIDEO_FRAME | 20 | VIDEO | One fragment of a video frame |

## Video transport

### Display target

The initial PSP display target is:

```text
Width:  480 pixels
Height: 272 pixels
```

The PSP LCD supports a maximum 480×272 display mode, and the PSP SDK exposes RGB 5:6:5 as a native 16-bit display format. citeturn0search0turn0search1

### Initial prototype format

The first video prototype uses **uncompressed RGB565** frames.

This is deliberately not the final CarPlay video format. Raw RGB565 removes codec/decoder complexity while the PSP-ICE transport and framebuffer path are developed and tested.

Pixel format:

```text
RGB565
16 bits/pixel
5 bits red
6 bits green
5 bits blue
```

A complete 480×272 RGB565 frame contains:

```text
480 × 272 × 2 = 261,120 bytes
```

A complete frame therefore cannot fit in one PSP-ICE UDP payload and must be fragmented.

### VIDEO_FRAME payload

Each `VIDEO_FRAME` message contains one fragment. The payload begins with a fixed 14-byte video fragment header in network byte order:

| Field | Size | Description |
|---|---:|---|
| Frame ID | 4 bytes | Identifies the complete video frame |
| Fragment ID | 2 bytes | Zero-based fragment index |
| Fragment count | 2 bytes | Total number of fragments in this frame |
| Width | 2 bytes | Frame width in pixels |
| Height | 2 bytes | Frame height in pixels |
| Pixel format | 1 byte | Pixel format identifier |
| Reserved | 1 byte | Must be zero in protocol version 1 |

Current pixel format identifiers:

| Value | Format |
|---:|---|
| `1` | RGB565 |

The remaining bytes in the payload are the fragment's pixel data.

For the initial raw 480×272 RGB565 prototype, implementations should keep each complete UDP payload at or below the protocol's 60,000-byte payload limit. A fragment therefore carries at most **59,986 bytes** of pixel data after the 14-byte video fragment header.

### Fragmentation example

A 261,120-byte RGB565 frame requires **5 fragments** at the maximum fragment payload size:

```text
FRAME 42
 ├── fragment 0
 ├── fragment 1
 ├── fragment 2
 ├── fragment 3
 └── fragment 4
```

`Frame ID` identifies the frame, while `Fragment ID` identifies the fragment within that frame.

The outer message `Sequence` remains the transport-level sequence number. It is independent of `Frame ID` and `Fragment ID`.

### Receiver behavior

The PSP receiver should:

1. Validate the normal PSP-ICE message header.
2. Validate the video fragment header.
3. Reject fragments with invalid dimensions or pixel format.
4. Store fragments belonging to the same `Frame ID`.
5. Detect missing fragments using `Fragment ID` and `Fragment count`.
6. Once every fragment is present, reconstruct the complete frame.
7. Display the newest complete frame.
8. Discard incomplete or obsolete frames rather than allowing latency to grow indefinitely.

The receiver must **not** wait indefinitely for a missing UDP fragment. A newer frame supersedes an older incomplete frame.

### Sender behavior

The gateway should:

1. Assign a new `Frame ID` to every generated frame.
2. Split the frame into fragments that fit inside the UDP payload limit.
3. Send all fragments with the same `Frame ID` and `Fragment count`.
4. Increment the normal message `Sequence` for each transmitted message.
5. Prefer dropping an old frame over building an unbounded transmit queue.

The initial implementation does not retransmit lost video fragments.

### Framebuffer output

The PSP can display RGB565 using the PSP display API's native 16-bit RGB 5:6:5 format. The PSP SDK documents the LCD mode as 480×272 and exposes `sceDisplaySetFrameBuf()` for selecting the displayed framebuffer. citeturn0search0

The first receiver implementation may use a double-buffered framebuffer to avoid displaying a partially reconstructed frame. The exact framebuffer/VRAM implementation is a PSP client detail and is not part of the network protocol.

### Future compressed video

The protocol is intentionally frame-oriented so the raw RGB565 prototype can later be replaced by compressed video without redesigning the entire PSP-ICE architecture.

Future formats may include:

- H.264
- another hardware/software-decoded video format suitable for the PSP
- intra-frame or delta-frame transport

The pixel format field and video message semantics may be extended in a future protocol version when compressed video is introduced.

## Input strategy

`INPUT_STATE` contains a monotonically increasing message sequence number and the current PSP button/analog state.

The input payload is exactly 4 bytes:

| Field | Size | Description |
|---|---:|---|
| Buttons | 2 bytes | Button bitmask, network byte order |
| Analog X | 1 byte | Horizontal analog value, `0–255` |
| Analog Y | 1 byte | Vertical analog value, `0–255` |

The analog stick is normally centered around `128` on each axis.

The gateway should process the newest valid input state. Missing UDP input packets are tolerated because a later state supersedes an older one.

## Control/session strategy

`HELLO`, `PING`, and `PONG` are used for basic endpoint discovery and connectivity testing.

The current prototype does not define authentication or encryption. Those concerns can be added when the transport is stable.

## Compatibility

Protocol versioning is mandatory from the first prototype so the PSP client and gateway can evolve independently.

Changes that alter an existing binary structure or message interpretation should increment the protocol version rather than silently changing the meaning of version 1 packets.
