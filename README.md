# PSP-ICE

**PSP In-Car Entertainment** — a phone-powered, CarPlay-inspired infotainment system that turns a Sony PSP into an in-car display, physical controller, and **audio playback device**.

> [!IMPORTANT]
> PSP-ICE is CarPlay-inspired; it is not an Apple CarPlay implementation or certification.

## Core architecture

The PSP is deliberately part of the audio path. The iPhone is the intended end-user host and audio source; it is planned to send digital audio and interface/control data to the PSP over local Wi-Fi. The PSP plays received audio through its 3.5 mm headphone output, which connects to the car stereo's AUX input.

```text
                 iPhone
       (planned companion app + audio source)
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

The PC/Linux gateway is an optional development and test tool. It is not required in the intended final car installation.

## Responsibilities

### iPhone (intended host; companion app not implemented)
- Planned companion app, application logic, pairing, metadata and media controls.
- Intended to supply audio to the PSP over the local Wi-Fi link.
- Planned UI data for the PSP's native 480×272 display.
- Receives PSP input and maps it to app/media actions.
- Spotify is an integration target, but the permitted and technically viable way to obtain audio for forwarding must be established. Spotify App Remote should not be assumed to expose decoded audio.

### PSP
- Runs the PSP-ICE homebrew client under custom firmware.
- Connects to a local Wi-Fi access point.
- Receives audio packets and plays PCM through its own audio hardware.
- Displays the intended infotainment UI at 480×272 (UI transport/rendering remains future work).
- Sends physical button/analog input to the host.

### Car
- Receives sound from the PSP's 3.5 mm headphone output through AUX.

## Connectivity

- **Intended end-user link:** iPhone Personal Hotspot ↔ PSP over local Wi-Fi.
- **Transport prototype:** UDP on port `39000`.
- **Development setup tested:** iPhone Personal Hotspot with both PSP and MacBook connected; the MacBook ran the test PCM sender and the PSP played the received audio.
- The MacBook/Linux gateway is a temporary development audio/test host; the iPhone companion app sender is not implemented yet.
- The current PSP client sends HELLO and input packets to limited broadcast address `255.255.255.255`; the Python PCM sender targets a manually supplied PSP IP.
- Discovery, pairing, iOS local-network permissions, reconnect behaviour and hotspot compatibility still need implementation/testing.

## Audio

The current tested transport format is:
- Signed 16-bit little-endian PCM
- Stereo
- 44,100 samples/second
- 256 audio frames per UDP packet
- 1,024-byte PCM payload plus the 14-byte PSP-ICE header
- About 5.8 ms of audio per packet

The PSP buffers incoming PCM and plays it using the PSP audio API. Its 3.5 mm output connects to the car AUX input.

**Verified milestone:** MacBook test sender → iPhone Personal Hotspot → PSP UDP receiver → PSP audio output. Clean playback was confirmed with the PSP close to the router and over the iPhone hotspot. In the hotspot test, the iPhone provided the network; it was not the audio sender.

## Interface direction

A dedicated music-first automotive dashboard inspired by BMW iDrive and Apple CarPlay, without copying proprietary assets or claiming official affiliation.

Design goals:
- Dark, high-contrast interface with restrained accents.
- Large, readable tiles and playback controls.
- Designed specifically for 480×272; do not shrink a phone UI onto the PSP.
- Physical PSP controls for navigation, selection, back and media actions.
- Now Playing view with album artwork, track title, artist, progress and controls.
- Clear connection and playback status.

The PSP-native infotainment UI and end-to-end display transport are not implemented yet.

## Initial feature scope

1. Reliable iPhone hotspot ↔ PSP local connection.
2. iPhone companion app and host-side session management.
3. PSP-native 480×272 interface.
4. PSP button/analog input sent to the phone.
5. Audio sent from the phone to the PSP and played through PSP 3.5 mm output.
6. PSP 3.5 mm → AUX → car stereo.
7. Music-service integration and now-playing metadata, subject to provider/platform APIs.
8. Reliable reconnect and clear disconnected states.

> [!NOTE]
> Navigation, calls, additional media providers and deeper vehicle integration are later possibilities.

## Development status

**Early prototype — PSP controller input and networked PSP audio playback have been demonstrated using a PC development sender/gateway.**

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
- [ ] PDF documentation and user manual

## Repository layout

- `psp/` — PSP homebrew client
- `gateway/` — optional PC/Linux/macOS development and test tools
- `protocol/` — transport specification
- `docs/` — project documentation and developer setup
- `tools/` — development/testing utilities
- `hardware/` — hardware notes and schematics (see documentation)
- `companion/` — planned iPhone companion app (not implemented yet)

## Documentation

Start with the [documentation index](docs/README.md), or go directly to the [development setup guide](docs/development-setup.md).

## Community

[PSP-ICE | Official Community ❄️](https://discord.gg/BaQJ3Zvm2V)
