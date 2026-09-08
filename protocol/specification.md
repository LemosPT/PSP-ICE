# PSP-ICE Transport Specification

## Goals

- Low latency
- Small packets suitable for PSP Wi-Fi
- Video may tolerate packet loss
- Input events must not be silently reordered
- Keep the protocol simple enough to implement in PSP homebrew

## Logical channels

| Channel | Direction | Purpose |
|---|---|---|
| CONTROL | Bidirectional | Session setup, capabilities, keepalive |
| VIDEO | Gateway → PSP | Display frames |
| INPUT | PSP → Gateway | Buttons and analog input |
| AUDIO | Gateway → PSP | Reserved for future digital audio transport |

## Initial message model

Every message begins with a small fixed header containing:

- Magic
- Protocol version
- Channel
- Message type
- Sequence number
- Payload length

The exact binary layout will be frozen after the PC prototype is tested.

## Video strategy

The initial prototype should use a frame-oriented transport so that the gateway can discard obsolete frames instead of building an ever-growing queue. Target display size is 480×272.

Codec and packetization will be selected after measuring PSP decoding performance and Wi-Fi loss.

## Input strategy

Input messages contain a monotonically increasing sequence number and the current PSP button/analog state. The gateway should process the newest valid state and detect dropped packets.

## Compatibility

Protocol versioning is mandatory from the first prototype so the PSP client and gateway can evolve independently.
