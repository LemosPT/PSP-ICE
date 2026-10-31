# PSP-ICE Transport Specification

## Goals

- Low latency on PSP-class Wi-Fi hardware.
- Small UDP datagrams.
- Audio packets are played in order and buffered to absorb short network jitter.
- Input remains responsive while audio is received.
- Display frames may tolerate packet loss and be superseded by newer frames.
- Keep the protocol simple enough to implement in PSP homebrew and an iOS companion app.

## Transport

PSP-ICE uses UDP for the prototype transport.

Default port:

```text
39000/UDP
```

UDP does not guarantee delivery, ordering or retransmission. Higher-level logic must detect missing/obsolete data where necessary. Audio sequence-loss handling and recovery are not yet implemented in the PSP receiver.

## Logical channels

| Channel | Value | Direction | Purpose |
|---|---:|---|---|
| CONTROL | 0 | Bidirectional | Session setup, capabilities, keepalive |
| VIDEO | 1 | Host → PSP | Display frames |
| INPUT | 2 | PSP → Host | Buttons and analog input |
| AUDIO | 3 | Host → PSP | Digital PCM audio |

## Message header

Every message begins with a fixed 14-byte header in network byte order (big endian):

| Field | Size | Description |
|---|---:|---|
| Magic | 4 bytes | ASCII `ICE1` |
| Version | 1 byte | Protocol version, currently `1` |
| Channel | 1 byte | Logical channel |
| Type | 1 byte | Message type |
| Flags | 1 byte | Message-specific flags; `0` unless defined otherwise |
| Sequence | 4 bytes | Transport message sequence number |
| Payload length | 2 bytes | Number of payload bytes following the header |

Total header size: **14 bytes**. The current maximum protocol payload is 60,000 bytes, although the PSP audio test uses much smaller datagrams.

## Message types

| Type | Value | Channel | Purpose |
|---|---:|---|---|
| HELLO | 1 | CONTROL | Announce a PSP-ICE endpoint |
| PING | 2 | CONTROL | Keepalive / connectivity test |
| PONG | 3 | CONTROL | Response to HELLO/PING |
| INPUT_STATE | 10 | INPUT | Current PSP button and analog state |
| VIDEO_FRAME | 20 | VIDEO | Video/display fragment (specified; implementation status must be checked) |
| AUDIO_DATA | 30 | AUDIO | Raw PCM audio packet |

## Audio transport (implemented prototype)

### PCM format

The current test sender and PSP receiver use this fixed PCM format:

| Property | Value |
|---|---|
| Encoding | Signed linear PCM |
| Sample width | 16-bit |
| Sample byte order | Little-endian |
| Channels | 2 (stereo) |
| Sample rate | 44,100 Hz |
| Frames per packet | 256 |
| Bytes per stereo frame | 4 |
| PCM payload size | 1,024 bytes |
| PSP-ICE header | 14 bytes |
| UDP datagram size | 1,038 bytes |
| Audio duration per packet | Approximately 5.805 ms |

Each AUDIO_DATA message payload is exactly 1,024 bytes. Samples are interleaved left/right signed 16-bit values. There is no additional audio-specific payload header.

The sender increments the outer 32-bit sequence number for each audio datagram. The PSP receiver currently validates the ICE1 header, channel, message type and expected payload length before passing PCM to the audio queue. It does not yet reject duplicate/out-of-order packets or count sequence gaps.

### PSP buffering and playback

The current PSP implementation:
- Receives UDP datagrams on port 39000.
- Drains multiple queued datagrams between controller input updates.
- Queues PCM for a separate audio worker thread.
- Uses 16 packet slots (nominal capacity approximately 92.9 ms).
- Waits for 4 packets before starting playback (nominal prebuffer approximately 23.2 ms).
- Plays 256-frame stereo blocks through the PSP audio API.

These are current prototype settings, not final performance guarantees. The queue uses shared volatile counters and needs a concurrency review. Underrun/overflow statistics, sequence tracking and improved jitter handling are planned hardening work.

### Verified end-to-end development test

The Python sender `gateway/src/audio_sender.py` generated a 440 Hz stereo PCM test tone and sent it over UDP to the PSP.

The test was performed with:
1. The PSP connected to the iPhone Personal Hotspot.
2. The MacBook also connected to the same iPhone Personal Hotspot.
3. The MacBook running the Python sender.
4. The PSP receiving and playing the PCM packets.

Clean playback was confirmed in this hotspot setup. The iPhone served as the Wi-Fi access point; the MacBook was still the audio sender. This test does not mean the iPhone companion app or iPhone-originated audio sender is already implemented.

The PSP's 3.5 mm headphone output is the intended physical audio connection to the car AUX input.

## Video transport

### Display target

The initial PSP display target is:

```text
Width:  480 pixels
Height: 272 pixels
```

### Proposed initial prototype format

The video design below is a transport proposal and must not be mistaken for a completed receiver implementation.

The first proposed video prototype uses uncompressed RGB565 frames.

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

A complete frame cannot fit in one UDP datagram and would need fragmentation.

### Proposed VIDEO_FRAME payload

Each VIDEO_FRAME message may contain one fragment. The proposed payload begins with a 14-byte video fragment header in network byte order:

| Field | Size | Description |
|---|---:|---|
| Frame ID | 4 bytes | Identifies the complete video frame |
| Fragment ID | 2 bytes | Zero-based fragment index |
| Fragment count | 2 bytes | Total number of fragments |
| Width | 2 bytes | Frame width |
| Height | 2 bytes | Frame height |
| Pixel format | 1 byte | Pixel format identifier |
| Reserved | 1 byte | Must be zero in protocol version 1 |

Proposed pixel format identifier: `1 = RGB565`.

The receiver design should validate each fragment, reconstruct complete frames, discard incomplete/obsolete frames, and display the newest complete frame. This proposal must be validated against PSP Wi-Fi throughput before implementation choices are finalized.

## Input strategy

INPUT_STATE contains the current PSP button/analog state. Its payload is exactly 4 bytes:

| Field | Size | Description |
|---|---:|---|
| Buttons | 2 bytes | Button bitmask, network byte order |
| Analog X | 1 byte | Horizontal analog value, 0–255 |
| Analog Y | 1 byte | Vertical analog value, 0–255 |

The analog stick is normally centered around 128 on each axis. Newer input state supersedes older state.

## Control/session strategy

HELLO, PING and PONG are used for basic endpoint discovery and connectivity testing. The current prototype does not define authentication or encryption. Pairing, session ownership, reconnect and capability negotiation remain future work.

## Compatibility

Protocol versioning is mandatory. Changes that alter an existing binary structure or message interpretation should increment the protocol version rather than silently changing version 1 packets.
