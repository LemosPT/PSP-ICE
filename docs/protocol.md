# PSP-ICE Protocol Notes

## Purpose

The PSP-ICE protocol carries the local connection between the intended iPhone companion app (host) and the PSP client. The PC/Linux gateway is a development/test peer; the wire format must remain platform-independent so the iPhone app can implement it.

Default development port: `39000/UDP`.

## Existing wire header

Defined in `psp/include/protocol.h` and `gateway/src/protocol.py`:

- Magic: `ICE1`
- Version: 1
- Header size: 14 bytes
- Fields: magic, version, channel, type, flags, sequence, payload length
- Multi-byte header numeric fields use network byte order (big endian).

The 14-byte header layout is:

| Field | Size |
|---|---:|
| Magic | 4 bytes |
| Version | 1 byte |
| Channel | 1 byte |
| Type | 1 byte |
| Flags | 1 byte |
| Sequence | 4 bytes |
| Payload length | 2 bytes |

## Channels

| Channel | Value | Direction | Purpose |
|---|---:|---|---|
| CONTROL | 0 | Bidirectional | Session and keepalive |
| VIDEO | 1 | Host → PSP | Display frames |
| INPUT | 2 | PSP → host | Buttons and analog state |
| AUDIO | 3 | Host → PSP | Digital PCM audio |

## Message identifiers

| Message | Value | Channel | Purpose |
|---|---:|---|---|
| HELLO | 1 | CONTROL | Announce endpoint |
| PING | 2 | CONTROL | Keepalive |
| PONG | 3 | CONTROL | Response |
| INPUT_STATE | 10 | INPUT | Current PSP button/analog state |
| VIDEO_FRAME | 20 | VIDEO | Video/display fragment |
| AUDIO_DATA | 30 | AUDIO | PCM audio packet |

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

The audio payload is raw PCM only; it does not contain a separate audio-specific header. The outer PSP-ICE sequence number increments once per datagram.

The PSP receiver currently accepts AUDIO_DATA packets only when the payload length is exactly 1,024 bytes. It queues the PCM for a separate audio playback thread. The current queue has 16 packet slots (about 92.9 ms) and begins playback after a 4-packet prebuffer (about 23.2 ms).

The packet format is implemented in `gateway/src/audio_sender.py` and the receive/playback path is implemented in `psp/src/main.c`, `psp/src/audio/audio.c` and `psp/include/audio.h`.

**Prototype limitations:** the PSP currently does not perform sequence-gap/loss accounting for audio, and the queue uses simple shared volatile state that still needs concurrency review. Underrun/overflow counters and robust queue synchronization are future hardening tasks. The 256-frame packet size and current buffering values are tested prototype parameters, not yet a final latency/quality guarantee.

## Input-state payload

The INPUT_STATE payload is exactly 4 bytes:

| Field | Size | Description |
|---|---:|---|
| Buttons | 2 bytes | Button bitmask, network byte order |
| Analog X | 1 byte | Horizontal analog value, 0–255 |
| Analog Y | 1 byte | Vertical analog value, 0–255 |

The analog stick is normally centered around 128 on each axis. Button bit definitions are maintained in `psp/include/protocol.h` and mirrored in `gateway/src/protocol.py`.

## Display transport

The UI target is the PSP's native 480×272 display. The companion app is intended to own the UI state and provide display data to the PSP. The exact representation (full frames, dirty rectangles, tiles or another compact update format) remains to be benchmarked on real PSP hardware.

The separate detailed video-fragment proposal is documented in `protocol/specification.md`; verify it against the current implementation before treating it as implemented.

## Intended traffic direction

Host → PSP:
- Session/control messages
- UI/video data
- AUDIO_DATA PCM packets

PSP → host:
- HELLO/session initiation as designed
- PING/PONG as appropriate
- Button and analog input state

Discovery, pairing, capability negotiation and reconnect state machine still need to be specified and implemented.

## Transport requirements

- UDP is the existing prototype transport.
- Validate iOS local-network permission behaviour and hotspot client routing.
- Do not assume UDP broadcast discovery works identically on every iOS hotspot configuration; provide a manual-address or pairing fallback if needed.
- Keep display updates, input events, session traffic and audio logically separate.
- Drop stale display frames rather than allowing a backlog to increase latency.
- Make input/session handling responsive and resilient to packet loss.
- Add sequence handling, bounds checks and payload validation before trusting received packets.
- Measure real PSP Wi-Fi throughput and latency before choosing frame encoding, bitrate or refresh rate.

## Compatibility

The wire protocol should be implementable in PSP homebrew C and iOS companion app code. Avoid platform-specific structs or native memory layouts on the wire. Serialize fields explicitly and define byte order and lengths.
