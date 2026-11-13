# PSP-ICE Architecture

## Product direction

PSP-ICE is a phone-powered, music-first infotainment system. The iPhone is the intended host and audio source; a Sony PSP provides the in-car display, physical controls, and **audio playback**.

The defining architectural requirement is that **the PSP remains in the audio path**. Audio travels from the phone to the PSP over local Wi-Fi, is played by the PSP, and leaves through the PSP's 3.5 mm headphone output to the car stereo's AUX input.

The final installation does not require a PC, Raspberry Pi, or Linux gateway. The existing PC/Linux gateway remains an optional development and protocol-testing tool.

PSP-ICE is CarPlay-inspired; it is not an Apple CarPlay implementation or certification.

## Current implementation boundary

The architecture below describes the intended product, not a fully completed system.

| Component/path | Current status |
|---|---|
| PSP UDP networking and controller-state sender | Implemented prototype |
| PC/Linux/macOS Python development gateway | Implemented |
| Python PCM test sender → PSP audio playback | Implemented and hardware-tested |
| iPhone Personal Hotspot as access point for PSP + MacBook test | Tested |
| iPhone companion app and iPhone-originated audio sender | Not implemented |
| Pairing, discovery, session ownership and reconnect | Not implemented as a complete flow |
| Local PSP dashboard scaffold | Implemented in source; local navigation and placeholder pages only |
| Phone-driven UI data and end-to-end display transport | Not implemented |
| Vehicle installation | Future work |

For reproducible build and test steps, see [Development Setup](development-setup.md). Packet-level status and known discrepancies are in [Protocol](protocol.md).

## High-level architecture

```text
                    iPhone
       ┌────────────────────────────────┐
       │ Planned PSP-ICE companion app  │
       │                                │
       │ • App/session logic            │
       │ • Intended audio source        │
       │ • Metadata and media controls  │
       │ • UI and display data          │
       └───────────────┬────────────────┘
                       ⇅
              Local Wi-Fi / UDP
          iPhone Personal Hotspot
             audio + UI + input
                       ⇅
       ┌───────────────┴────────────────┐
       │ Sony PSP                       │
       │ PSP-ICE homebrew client        │
       │                                │
       │ • Receive and play PCM audio   │
       │ • Intended 480×272 interface   │
       │ • Read physical controls       │
       │ • Send input to host           │
       └───────────────┬────────────────┘
                       │ 3.5 mm headphone output
                       │
                    AUX cable
                       │
                 Car stereo
```

The phone and PSP have distinct intended responsibilities:
- The iPhone owns application logic, media-service integration, metadata, session management and the upstream audio source.
- The PSP receives and plays audio, is intended to render the infotainment interface, and returns physical input.
- The car receives sound from the PSP's 3.5 mm output.

## 1. iPhone companion app (planned)

The companion app is the intended primary host for the end-user system. It is not implemented yet.

Planned responsibilities:
- Manage PSP pairing, connection status and reconnect attempts.
- Supply audio to the PSP over the local Wi-Fi connection.
- Provide track metadata, artwork and playback state.
- Receive PSP button/analog events and map them to interface and media actions.
- Provide UI data designed for the PSP's native 480×272 display.
- Integrate with music providers within their current SDK/API, account, authentication and iOS restrictions.

### Audio-source constraint

The iPhone-to-PSP audio sender is **not implemented yet**. The current sender is a Python development tool running on a MacBook/Linux host and generating a test tone.

Spotify integration is an open technical constraint: Spotify App Remote is primarily a playback-control/metadata interface and must not be assumed to expose decoded PCM audio for arbitrary forwarding. The project must identify and test a legitimate, technically viable audio source path before committing to a Spotify audio-capture design.

### iOS networking constraints to validate

The user enables Personal Hotspot in iOS settings, then connects the PSP and companion host to it. Do not assume an ordinary third-party iOS app can silently enable Personal Hotspot.

The app will need appropriate iOS local-network permission. Test hotspot routing, UDP behaviour, app lifecycle and reconnect behaviour on real iPhones. If automatic discovery is unreliable, provide a manual pairing/address fallback.

## 2. PSP client

Target hardware: Sony PSP-2000 family (initial test target PSP-2004) running custom firmware.

Current implemented responsibilities:
- Join a Wi-Fi access point using the PSP network APIs.
- Bind a UDP socket on port 39000.
- Send a HELLO packet and controller-state updates to the limited broadcast address `255.255.255.255`.
- Receive and buffer digital audio packets.
- Play audio through the PSP audio hardware.
- Output sound through the PSP's 3.5 mm headphone jack to the car AUX input.

Planned responsibilities:
- Discover/pair with the companion app or connect using a fallback address.
- Render the infotainment UI at native 480×272 resolution.
- Show a clear disconnected/reconnecting state.

The current client has a working UDP audio receive path and a separate controller-input transmit path. Audio and controls have been demonstrated simultaneously. The broadcast destination is a prototype shortcut, not a finished discovery mechanism.

## 3. Wi-Fi transport and protocol

The existing PSP-ICE UDP protocol is the starting point for the direct phone-to-PSP link.

Default development port: `39000/UDP`.

Logical channels:
- **Control:** currently basic HELLO/PING/PONG test messages; pairing, capability negotiation, disconnect and reconnect semantics remain future work.
- **Display:** intended UI/video frames or display updates from phone to PSP; end-to-end receiver is not implemented.
- **Input:** button/analog events from PSP to host; current PSP sender exists.
- **Audio:** PCM audio packets from development host to PSP; current test path exists.

Keep transport responsibilities separate. Input/session handling should remain responsive while audio is being received and played.

## 4. Audio architecture

### Required audio route

```text
iPhone audio source / companion app (planned)
              │
              │ local Wi-Fi / UDP PCM
              ▼
        PSP audio receiver
              │
        PCM queue/buffer
              │
        PSP audio hardware
              │
        3.5 mm headphone output
              │
           AUX cable
              │
          Car stereo
```

The PSP is not merely a display terminal: it receives and plays the audio stream. Do not route the phone's audio directly to the car as the baseline architecture.

### Current prototype format

- Signed 16-bit little-endian PCM.
- Stereo.
- 44,100 Hz.
- 256 frames per packet.
- 1,024 PCM bytes per packet, plus the 14-byte transport header.
- Each packet represents approximately 5.8 ms of audio.
- PSP receiver currently uses a 16-packet queue and a 4-packet prebuffer.

The audio queue is protected by a PSP kernel semaphore mutex. The audio module exposes received/played/dropped packet counts, playback errors and queue depth. The main loop separately tracks sequence gaps, duplicates and out-of-order datagrams and displays estimated packet loss on the home dashboard. These counters are diagnostic only: duplicates and out-of-order packets are still passed to the audio queue, and a dedicated underrun counter is not implemented. Physical-device stress testing and tuning remain necessary.

### Verified audio milestone

The Python test sender ran on a MacBook connected to the iPhone Personal Hotspot. The PSP was also connected to that hotspot. The MacBook sent test PCM over UDP to the PSP, and the PSP played it cleanly. The same audio receiver had previously been tested over the home router; crackling at greater distance was eliminated when the PSP was brought close to the router.

This validates the network-to-PSP playback path and the iPhone hotspot network arrangement. It does **not** yet validate the iPhone as the audio sender or Spotify audio integration.

## 5. UI and interaction design

### Display constraints
- Native PSP resolution: 480×272.
- Design at target resolution from the beginning.
- Use large typography, high contrast, simple icons and clear focus states.
- Avoid dense layouts and tiny touch targets; the PSP is controlled with physical buttons.

A local PSP-native GU dashboard scaffold is present in the current source. It draws six selectable tiles and a static media strip, supports D-pad navigation, Cross to open a section and Circle to return home, and displays estimated audio packet loss when sequence data is available. App pages are placeholders; media metadata, playback controls, live Wi-Fi/Bluetooth telemetry and phone-driven display updates are not implemented. This scaffold is source-verified, not independently hardware-validated.

### Initial screens

**Home dashboard**
- Music
- Now Playing
- Navigation (future/placeholder)
- Phone (future/placeholder)
- Podcasts (future/placeholder)
- Settings / connection

**Now Playing**
- Album artwork
- Track title and artist
- Playback progress
- Play/pause, previous, next
- Optional shuffle/repeat state when supported

**Connection state**
- Connected/disconnected indicator
- Hotspot/Wi-Fi status where available
- Reconnecting message and retry behaviour

### Input mapping

The PSP D-pad and analog stick are intended to navigate focus. Cross selects; Circle goes back; Start/Select may be assigned to system shortcuts. L/R can be mapped to previous/next track when appropriate. Final mappings should be configurable and documented.

## 6. Optional PC/Linux development gateway

The existing `gateway/` code remains useful for:
- Testing UDP protocol changes without the iPhone app.
- Sending test PCM audio to the PSP.
- Inspecting input packets and session traffic.
- Developing the PSP client independently of iOS.
- Reproducing audio tests and measuring performance.

It is a development tool, not a required component of the intended in-car installation.

## 7. Development phases

See the [Development Roadmap](roadmap.md) for prioritized work and status.

## Design principles

- **The PSP must remain in the audio path.**
- iPhone is the intended app host and audio source.
- PSP is the display, physical controller and audio playback endpoint.
- No PC is required for normal end-user operation.
- The PC/Linux gateway is optional development infrastructure.
- Dedicated music-first UI, not phone-screen mirroring.
- Keep the PSP client lightweight.
- Separate display, input, session and audio responsibilities.
- Be realistic about PSP Wi-Fi throughput and iOS networking restrictions.
