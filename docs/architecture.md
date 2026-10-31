# PSP-ICE Architecture

## Product direction

PSP-ICE is a phone-powered, music-first infotainment system. The iPhone is the intended host and audio source; a Sony PSP provides the in-car display, physical controls, and **audio playback**.

The defining architectural requirement is that **the PSP remains in the audio path**. Audio travels from the phone to the PSP over local Wi-Fi, is played by the PSP, and leaves through the PSP's 3.5 mm headphone output to the car stereo's AUX input.

The final installation does not require a PC, Raspberry Pi, or Linux gateway. The existing PC/Linux gateway remains an optional development and protocol-testing tool.

PSP-ICE is CarPlay-inspired; it is not an Apple CarPlay implementation or certification.

## High-level architecture

```text
                    iPhone
       ┌────────────────────────────────┐
       │ PSP-ICE companion app          │
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
       │ • Render 480×272 interface     │
       │ • Read physical controls       │
       │ • Send input to iPhone         │
       └───────────────┬────────────────┘
                       │ 3.5 mm headphone output
                       │
                    AUX cable
                       │
                 Car stereo
```

The phone and PSP have distinct responsibilities:
- The iPhone owns application logic, media-service integration, metadata, session management and the upstream audio source.
- The PSP receives and plays audio, renders the infotainment interface, and returns physical input.
- The car receives sound from the PSP's 3.5 mm output.

## 1. iPhone companion app

The companion app is the intended primary host for the end-user system.

Responsibilities:
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

Responsibilities:
- Join the iPhone Personal Hotspot over Wi-Fi.
- Discover/pair with the companion app or connect using a fallback address.
- Receive and buffer digital audio packets.
- Play audio through the PSP audio hardware.
- Output sound through the PSP's 3.5 mm headphone jack to the car AUX input.
- Render the infotainment UI at native 480×272 resolution.
- Send button and analog-stick events to the phone.
- Show a clear disconnected/reconnecting state.

The current client has a working UDP audio receive path and a separate controller-input transmit path. Audio and controls have been demonstrated simultaneously.

## 3. Wi-Fi transport and protocol

The existing PSP-ICE UDP protocol is the starting point for the direct phone-to-PSP link.

Default development port: `39000/UDP`.

Logical channels:
- **Session/control:** hello, pairing, capability negotiation, ping/pong, disconnect and reconnect.
- **Display:** UI/video frames or display updates from phone to PSP.
- **Input:** button/analog events from PSP to phone.
- **Audio:** PCM audio packets from phone/host to PSP.

Keep transport responsibilities separate. Input/session handling should remain responsive while audio is being received and played.

## 4. Audio architecture

### Required audio route

```text
iPhone audio source / companion app
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

The PSP is not merely a display terminal: it receives and decodes/plays the audio stream. Do not route the phone's audio directly to the car as the baseline architecture.

### Current prototype format

- Signed 16-bit little-endian PCM.
- Stereo.
- 44,100 Hz.
- 256 frames per packet.
- 1,024 PCM bytes per packet, plus the 14-byte transport header.
- Each packet represents approximately 5.8 ms of audio.
- PSP receiver currently uses a 16-packet queue and a 4-packet prebuffer.

The current queue implementation is a prototype and still needs concurrency, underrun, overflow and sequence-loss diagnostics.

### Verified audio milestone

The Python test sender ran on a MacBook connected to the iPhone Personal Hotspot. The PSP was also connected to that hotspot. The MacBook sent test PCM over UDP to the PSP, and the PSP played it cleanly. The same audio receiver had previously been tested over the home router; crackling at greater distance was eliminated when the PSP was brought close to the router.

This validates the network-to-PSP playback path and the iPhone hotspot network arrangement. It does **not** yet validate the iPhone as the audio sender or Spotify audio integration.

## 5. UI and interaction design

### Display constraints
- Native PSP resolution: 480×272.
- Design at target resolution from the beginning.
- Use large typography, high contrast, simple icons and clear focus states.
- Avoid dense layouts and tiny touch targets; the PSP is controlled with physical buttons.

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

The PSP D-pad and analog stick navigate focus. Cross selects; Circle goes back; Start/Select may be assigned to system shortcuts. L/R can be mapped to previous/next track when appropriate. Final mappings should be configurable and documented.

## 6. Optional PC/Linux development gateway

The existing `gateway/` code remains useful for:
- Testing UDP protocol changes without the iPhone app.
- Sending test PCM audio to the PSP.
- Inspecting input packets and session traffic.
- Developing the PSP client independently of iOS.
- Reproducing audio tests and measuring performance.

It is a development tool, not a required component of the intended in-car installation.

## 7. Development phases

1. Keep the PC gateway ↔ PSP network prototype working.
2. Harden audio receive/playback with sequence, underrun and overflow diagnostics.
3. Validate the iPhone Personal Hotspot ↔ PSP connection under realistic conditions.
4. Build the iPhone companion app shell and local-network permissions.
5. Implement the iPhone-originated audio sender using a validated audio source.
6. Specify discovery, pairing, session and reconnect behaviour.
7. Build the PSP-native 480×272 interface and display transport.
8. Integrate metadata and media controls within provider/platform limits.
9. Test PSP 3.5 mm output → AUX → car stereo in the vehicle.
10. Refine automotive UI, mounting and usability.

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
