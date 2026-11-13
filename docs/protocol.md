# PSP-ICE Protocol Notes

## Purpose and status

The PSP-ICE protocol is the platform-independent UDP wire format intended for communication between the future iPhone companion app (host) and the PSP client. The PC/Linux gateway is currently a development and test peer.

- **Implemented:** v1 packet header, HELLO/PING/PONG test messages, PSP input-state transmission, and PCM audio receive/playback.
- **Tested:** PC-to-PC protocol round trip and development-host-to-PSP PCM playback, including a test using the iPhone Personal Hotspot as the access point.
- **Not implemented:** production discovery/pairing, authentication, session ownership, reconnect state machine, iPhone audio sender and end-to-end display transport.
- **Proposed:** video fragment payload described in [the transport specification](../protocol/specification.md).

Default development port: `39000/UDP`.

## Existing wire header

The header is defined in `psp/src/main.c` and `gateway/src/protocol.py`; constants are in `psp/include/protocol.h`.

- Magic: ASCII `ICE1`
- Version: `1`
- Header size: 14 bytes
- Fields: magic, version, channel, type, flags, sequence, payload length
- Multi-byte header numeric fields use network byte order (big endian).

| Field | Size |
|---|---:|
| Magic | 4 bytes |
| Version | 1 byte |
| Channel | 1 byte |
| Type | 1 byte |
| Flags | 1 byte |
| Sequence | 4 bytes |
| Payload length | 2 bytes |

The payload length describes only the bytes following the 14-byte header. The Python codec currently accepts payloads up to 60,000 bytes; this is a Python codec limit, not a demonstrated end-to-end PSP-ICE payload limit. The PSP audio receive buffer is 1,200 bytes and accepts the current 1,038-byte audio datagram; its outbound control/input buffer is smaller.

## Channels

| Channel | Value | Direction | Purpose |
|---|---:|---|---|
| CONTROL | 0 | Bidirectional | Basic endpoint announcement and keepalive |
| VIDEO | 1 | Host → PSP | Proposed display-frame transport |
| INPUT | 2 | PSP → host | Buttons and analog state |
| AUDIO | 3 | Host → PSP | Digital PCM audio |

## Message identifiers

| Message | Value | Channel | Status |
|---|---:|---|---|
| HELLO | 1 | CONTROL | Implemented for basic endpoint test |
| PING | 2 | CONTROL | Implemented in Python gateway/client |
| PONG | 3 | CONTROL | Implemented in Python gateway/client |
| INPUT_STATE | 10 | INPUT | Implemented by PSP sender and Python decoder |
| VIDEO_FRAME | 20 | VIDEO | Identifier exists; end-to-end display transport is not implemented |
| AUDIO_DATA | 30 | AUDIO | Implemented for the PCM test path |

## Audio data format (implemented prototype)

The current PSP receiver and Python test sender use:

- Signed 16-bit PCM samples.
- Little-endian sample byte order.
- Stereo (2 channels).
- 44,100 samples/second.
- 256 sample frames per packet.
- 4 bytes per stereo frame.
- 1,024-byte audio payload per packet.
- 14-byte PSP-ICE header, making a 1,038-byte UDP datagram.
- Approximately 5.805 ms of audio per packet.

The audio payload is raw interleaved PCM only; it has no separate audio-specific header. The outer 32-bit sequence number increments once per datagram.

The PSP receiver accepts AUDIO_DATA packets with an exactly 1,024-byte payload and queues the PCM for playback. The current queue has 16 packet slots (about 92.9 ms) and starts playback after a 4-packet prebuffer (about 23.2 ms).

The packet format is implemented in `gateway/src/audio_sender.py`; the receive/playback path is implemented in `psp/src/main.c`, `psp/src/audio/audio.c` and `psp/include/audio.h`.

**Current diagnostics:** the PSP main loop tracks sequence gaps, duplicate sequence numbers and out-of-order sequence numbers, and displays an estimated loss percentage on the local dashboard. The audio module exposes received/played/dropped packet counts, playback errors and queue depth. Queue state and packet copies are protected by a PSP kernel semaphore mutex; when full, the queue drops its oldest packet and increments the drop counter.

**Prototype limitations:** sequence diagnostics do not yet reject duplicates or reorder out-of-order packets before queueing. A dedicated playback-underrun counter is not implemented. Packet size and buffering values are prototype settings, not a final latency or quality guarantee.

## Input-state payload

INPUT_STATE is exactly 4 bytes:

| Field | Size | Description |
|---|---:|---|
| Buttons | 2 bytes | Button bitmask, network byte order |
| Analog X | 1 byte | Horizontal analog value, 0–255 |
| Analog Y | 1 byte | Vertical analog value, 0–255 |

The analog stick is normally centered around 128 on each axis.

### Current PSP button bit assignments

The PSP C implementation in `psp/include/protocol.h` currently assigns these bits:

| Bit | Button |
|---:|---|
| 0 | SELECT |
| 1 | START |
| 2 | UP |
| 3 | RIGHT |
| 4 | DOWN |
| 5 | LEFT |
| 6 | L |
| 7 | R |
| 8 | TRIANGLE |
| 9 | CIRCLE |
| 10 | CROSS |
| 11 | SQUARE |
| 12 | HOME |
| 13 | HOLD |

### Canonical v1 button mapping

The Python gateway constants have been aligned with the PSP C assignments. The shared v1 mapping is:

| Bit | Button |
|---:|---|
| 0 | SELECT |
| 1 | START |
| 2 | UP |
| 3 | RIGHT |
| 4 | DOWN |
| 5 | LEFT |
| 6 | L |
| 7 | R |
| 8 | TRIANGLE |
| 9 | CIRCLE |
| 10 | CROSS |
| 11 | SQUARE |
| 12 | HOME |
| 13 | HOLD |

A Python regression test checks the key shared assignments and diagnostic names. Keep `gateway/src/protocol.py` and `psp/include/protocol.h` synchronized when changing this mapping.

## Display transport

The UI target is the PSP's native 480×272 display. The companion app is intended to own UI state and provide display data to the PSP. The exact representation (full frames, dirty rectangles, tiles or another compact update format) remains to be benchmarked on real PSP hardware.

The detailed RGB565 video-fragment design in `protocol/specification.md` is a proposal, not a completed receiver implementation.

## Current network behaviour

- The PSP client binds UDP port 39000.
- The PSP currently sends HELLO and INPUT_STATE packets to `255.255.255.255:39000` (limited IPv4 broadcast).
- The Python gateway listens on the configured address (default `0.0.0.0:39000`) and can receive those packets when the local network forwards broadcast traffic.
- The Python PCM test sender sends AUDIO_DATA directly to a manually supplied PSP IP address.
- No completed discovery, pairing, capability negotiation, authentication or reconnect state machine exists.

Do not assume broadcast behaves identically on every access point or phone hotspot. A manual-address/pairing fallback and a defined connection lifecycle are still required.

## Transport requirements and future work

- UDP does not guarantee delivery, ordering or retransmission.
- Keep display updates, input events, session traffic and audio logically separate.
- Drop stale display frames rather than allowing a backlog to increase latency.
- Make input/session handling responsive and resilient to packet loss.
- Add sequence handling, bounds checks and payload validation before trusting received packets.
- Measure real PSP Wi-Fi throughput and latency before choosing frame encoding, bitrate or refresh rate.
- Define pairing, session ownership, reconnect behaviour and capability negotiation before companion-app integration.

## Compatibility

The wire protocol must remain implementable in PSP homebrew C and iOS companion-app code. Avoid platform-specific structs or native memory layouts on the wire. Serialize fields explicitly and define byte order and lengths. Changes that alter an existing binary structure or message interpretation should increment the protocol version rather than silently changing v1.
