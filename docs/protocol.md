# PSP-ICE Protocol Notes

## Purpose

The PSP-ICE protocol carries the local connection between the phone companion app (the intended end-user host) and the PSP client.

The existing PC/Linux gateway remains a development/test peer. The protocol should not depend on Linux-specific behaviour so the iPhone app can implement the same wire format.

## Existing wire header

Defined in psp/include/protocol.h:

- Magic: ICE1
- Version: 1
- Header size: 14 bytes
- Fields: magic, version, channel, type, flags, sequence, payload length
- Multi-byte numeric fields use network byte order.

The protocol currently defines these channel identifiers:
- Control: 0
- Video: 1
- Input: 2
- Audio: 3

The initial phone-direct AUX design does not need to transport audio to the PSP. Keep the audio channel reserved for possible future use; do not implement audio streaming as a first milestone.

## Existing message identifiers

- HELLO: 1
- PING: 2
- PONG: 3
- INPUT_STATE: 10
- VIDEO_FRAME: 20

Input-state payload currently contains:
- Buttons (uint16, network byte order)
- Analog X (uint8)
- Analog Y (uint8)

Button bit definitions are maintained in psp/include/protocol.h.

## Intended traffic direction

Phone → PSP:
- Session/control responses and capability negotiation
- UI/video frame data

PSP → phone:
- HELLO/session initiation as designed
- PING/PONG as appropriate
- Button and analog input state

The exact handshake, discovery, pairing, capability negotiation, and reconnect state machine still need to be specified and implemented.

## Transport design requirements

- UDP is the existing starting point.
- Validate iOS local-network permission behaviour and hotspot client routing.
- Do not assume UDP broadcast discovery works identically on every iOS hotspot configuration; provide a manual-address or pairing fallback if needed.
- Keep display updates and input events logically separate.
- Drop stale display frames rather than allowing a backlog to increase latency.
- Make input/session handling responsive and resilient to packet loss.
- Add sequence handling, bounds checks, and payload validation before trusting received packets.
- Measure real PSP Wi-Fi throughput and latency before choosing frame encoding, bitrate, or refresh rate.

## UI rendering strategy

The companion app owns the interface state and renders for the PSP's 480×272 target. The exact wire representation (full frames, dirty rectangles, tiles, or another compact update format) remains an implementation decision to be benchmarked.

The initial UI is static/low-motion and music-focused. Prioritize responsiveness and legibility over animation or high frame rate.

## Compatibility

The wire protocol should be implementable in:
- PSP homebrew C
- iOS companion app code

Avoid platform-specific structs or native memory layouts on the wire. Serialize fields explicitly and define byte order and lengths.
