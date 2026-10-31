# PSP-ICE

**PSP In-Car Entertainment** — a phone-powered, CarPlay-inspired infotainment system that turns a Sony PSP into the display and physical controller for a car music interface.

## Product direction

The intended end-user setup is **iPhone → Wi-Fi → PSP**, with no PC or Linux gateway required in the car.

The iPhone companion app is the system's host:
- Renders a dedicated infotainment interface designed for the PSP's 480×272 display.
- Connects to music services, starting with Spotify as the first integration target.
- Sends the interface to the PSP over the local Wi-Fi connection.
- Receives PSP button/analog input and translates it into app/media actions.
- Manages pairing, connection state, playback metadata, and settings.

The PSP is the display and physical controller:
- Runs the PSP-ICE homebrew client under custom firmware.
- Connects to the iPhone's Personal Hotspot.
- Receives and displays the phone-rendered interface.
- Sends button/analog events back to the companion app.

## Connectivity

- **Primary link:** direct local Wi-Fi between the iPhone and PSP.
- **Setup target:** the user enables the iPhone Personal Hotspot; the PSP joins it.
- **Firmware:** ARK-4 and ARK-5 Wi-Fi connectivity with iPhone hotspots is part of the project's target setup.
- **Transport:** PSP-ICE's existing UDP-based protocol is the starting point. Discovery, pairing, reconnect behaviour, and iOS local-network permissions still need implementation and real-device testing.
- A PC/Linux gateway remains useful as an optional development and debugging tool, not a required end-user component.

## Audio

The selected initial audio design is **phone-direct AUX (Option B)**:

```text
iPhone companion app / Spotify playback
        │
        ├── Wi-Fi ──→ PSP (display + controls only)
        │
        └── Lightning-to-3.5 mm audio adapter
                    │
                    └── AUX ──→ car stereo
```

The PSP is not in the audio path. On an iPhone with no headphone jack (such as iPhone 13), the car AUX connection requires a compatible Lightning-to-3.5 mm audio adapter and AUX cable. Bluetooth audio may be considered as an optional future output, but it is not the selected baseline.

## Interface direction

A dedicated music-first dashboard inspired by the visual language of BMW iDrive and Apple CarPlay, without copying either product or claiming Apple CarPlay compatibility.

Design goals:
- Dark, high-contrast automotive interface with restrained blue accents.
- Large, readable tiles and playback controls.
- Optimized specifically for 480×272; do not shrink a phone UI onto the PSP.
- Physical PSP controls for navigation, selection, back, and media actions.
- Home screen with music, navigation/phone placeholders, podcasts, and settings as the product evolves.
- A dedicated Now Playing view with album artwork, track title, artist, progress, and playback controls.
- Clear connection status and graceful disconnected/reconnecting states.

## Initial feature scope

1. iPhone companion app can connect to the PSP over the iPhone hotspot.
2. PSP-ICE displays a phone-rendered home screen.
3. PSP buttons navigate the interface.
4. Spotify integration provides playback control and now-playing metadata where permitted by Spotify's current SDK/API rules.
5. iPhone sends audio directly to the car over AUX.
6. Reliable reconnect and a clear disconnected state.

Navigation, calls, additional media providers, Bluetooth audio output, and deeper vehicle integration are later possibilities—not requirements for the first milestone.

## Development approach

- Keep the existing PC/Linux gateway available for protocol development, packet inspection, and PSP testing.
- Move the intended end-user host role to the iPhone companion app.
- Keep the PSP client lightweight.
- Separate UI/video transport, input/control, discovery/session management, and audio output.
- Do not finalize a video codec or bitrate until measured on real PSP Wi-Fi hardware.

## Repository layout

- `psp/` — PSP homebrew client
- `gateway/` — optional Linux development/test gateway
- `protocol/` — PSP ↔ host protocol
- `docs/` — architecture and hardware documentation
- `tools/` — development/testing utilities
- `hardware/` — car-side hardware notes and schematics
- `companion/` — planned iPhone companion app (not implemented yet)

## Project status

Early development / architecture phase.

- [x] PSP ↔ gateway networking prototype
- [ ] Direct iPhone ↔ PSP connection and discovery
- [ ] PSP video/UI receiver
- [ ] PSP input transport to iPhone app
- [ ] iPhone companion app and 480×272 UI renderer
- [ ] Spotify integration
- [ ] iPhone-to-car AUX audio setup documentation/testing
- [ ] Reconnect and session management
- [ ] Car installation / mounting

The project is **CarPlay-inspired**, not an implementation or certification of Apple's CarPlay system.

[PSP-ICE | Official Community ❄️](https://discord.gg/BaQJ3Zvm2V)
