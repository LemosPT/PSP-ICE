# PSP-ICE

**PSP In-Car Entertainment** — a phone-powered, CarPlay-inspired infotainment system that turns a Sony PSP into an in-car display, physical controller, and **audio playback device**.

PSP-ICE is CarPlay-inspired; it is not an Apple CarPlay implementation or certification.

## Core architecture

The PSP is deliberately part of the audio path. The iPhone is the intended end-user host and audio source; it sends digital audio and interface/control data to the PSP over local Wi-Fi. The PSP plays the received audio through its 3.5 mm headphone output, which connects to the car stereo's AUX input.

```text
                  iPhone
       (companion app + audio source)
                    ⇅
             Local Wi-Fi / UDP
          (audio, UI data, controls)
                    ⇅
              Sony PSP
       (display + physical controls
            + audio playback)
                    │
             3.5 mm output
                    │
                 AUX cable
                    │
               Car stereo
```

The PC/Linux gateway is an optional development and test tool. It is not required in the final car installation.

## Responsibilities

### iPhone (intended host)
- Companion app, application logic, pairing, metadata and media controls.
- Supplies audio to the PSP over the local Wi-Fi link.
- Renders or provides UI data for the PSP's native 480×272 display.
- Receives PSP input and maps it to app/media actions.
- Spotify is an integration target, but the permitted and technically viable way to obtain audio for forwarding must be established. Spotify App Remote should not be assumed to expose decoded audio.

### PSP
- Runs the PSP-ICE homebrew client under custom firmware.
- Connects to the iPhone's Personal Hotspot.
- Receives audio packets and plays PCM through its own audio hardware.
- Displays the infotainment UI at 480×272.
- Sends physical button/analog input to the host.

### Car
- Receives audio from the PSP's 3.5 mm headphone output through AUX.

## Connectivity

- **Intended end-user link:** iPhone Personal Hotspot ↔ PSP over local Wi-Fi.
- **Transport prototype:** UDP on port `39000`.
- **Development setup tested:** iPhone Personal Hotspot with both PSP and MacBook connected; the MacBook ran the test PCM sender and the PSP played the received audio cleanly.
- The MacBook/Linux gateway is only a temporary audio/test host in development; the iPhone companion app sender is not implemented yet.
- Discovery, pairing, iOS local-network permissions, reconnect behaviour and hotspot compatibility still need further implementation/testing.

## Audio

The current tested transport format is:
- Signed 16-bit little-endian PCM
- Stereo
- 44,100 samples/second
- 256 audio frames per UDP packet
- 1,024-byte PCM payload plus the 14-byte PSP-ICE header
- 5.8 ms of audio per packet (approximately)

The PSP buffers incoming PCM and plays it using the PSP audio API. Its 3.5 mm output is connected to the car AUX input.

**Verified milestone:** MacBook test sender → iPhone Personal Hotspot → PSP UDP receiver → PSP audio output. Clean playback was confirmed with the PSP close to the router and again over the iPhone hotspot. The iPhone was providing the network in the latter test; it was not yet acting as the audio sender.

## Interface direction

A dedicated music-first automotive dashboard inspired by BMW iDrive and Apple CarPlay, without copying proprietary assets or claiming official affiliation.

Design goals:
- Dark, high-contrast interface with restrained accents.
- Large, readable tiles and playback controls.
- Designed specifically for 480×272; do not shrink a phone UI onto the PSP.
- Physical PSP controls for navigation, selection, back and media actions.
- Now Playing view with album artwork, track title, artist, progress and controls.
- Clear connection and playback status.

## Initial feature scope

1. Reliable iPhone hotspot ↔ PSP local connection.
2. iPhone companion app and host-side session management.
3. PSP-native 480×272 interface.
4. PSP button/analog input sent to the phone.
5. Audio sent from the phone to the PSP and played through PSP 3.5 mm output.
6. PSP 3.5 mm → AUX → car stereo.
7. Music-service integration and now-playing metadata, subject to provider/platform APIs.
8. Reliable reconnect and clear disconnected states.

Navigation, calls, additional media providers and deeper vehicle integration are later possibilities.

## Development status

**Early prototype — bidirectional controls and networked PSP audio playback demonstrated.**

- [x] PSP ↔ development host UDP networking
- [x] PSP controller input sent to development host
- [x] PSP UDP PCM audio receiver and buffered playback
- [x] Development PCM test sender
- [x] Clean test playback over the iPhone Personal Hotspot network (MacBook used as sender)
- [ ] Native iPhone companion app
- [ ] iPhone-originated audio sender
- [ ] Validate a viable/legal audio source and Spotify integration constraints
- [ ] PSP-native infotainment UI
- [ ] UI/display transport
- [ ] Pairing, discovery and reconnect state machine
- [ ] Car installation and mounting

## Repository layout

- `psp/` — PSP homebrew client
- `gateway/` — optional PC/Linux development and test tools
- `protocol/` — transport specification
- `docs/` — architecture, hardware and protocol notes
- `tools/` — development/testing utilities
- `hardware/` — car-side hardware notes and schematics
- `companion/` — planned iPhone companion app (not implemented yet)

[PSP-ICE | Official Community ❄️](https://discord.gg/BaQJ3Zvm2V)
