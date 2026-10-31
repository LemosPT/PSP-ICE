# PSP-ICE Architecture

## Product direction

PSP-ICE is a phone-powered, music-first infotainment system. A dedicated iPhone companion app renders a CarPlay-inspired/BMW iDrive-inspired interface for the Sony PSP. The PSP acts as the in-car display and physical controller.

The intended end-user architecture does **not** require a PC, Raspberry Pi, or Linux gateway in the car. The existing Linux gateway remains useful as an optional development and protocol-testing tool.

PSP-ICE is CarPlay-inspired; it is not an Apple CarPlay implementation or certification.

## High-level architecture

```text
                         iPhone
              ┌──────────────────────────┐
              │ PSP-ICE companion app    │
              │                          │
              │ • UI renderer (480×272)  │
              │ • Spotify integration   │
              │ • Session/discovery     │
              │ • Input/action mapping  │
              └────────────┬─────────────┘
                           │
                 Local Wi-Fi / UDP
                 iPhone Personal Hotspot
                           │
              ┌────────────▼─────────────┐
              │ Sony PSP-2004            │
              │ PSP-ICE homebrew client  │
              │                          │
              │ • Receive UI/video       │
              │ • Display at 480×272     │
              │ • Send physical input   │
              └──────────────────────────┘

 iPhone audio output ── Lightning-to-3.5 mm adapter ── AUX ── car stereo
```

The phone and PSP have distinct responsibilities:
- The phone owns application logic, music-service integration, UI state, rendering, and audio playback.
- The PSP receives the rendered interface and returns user input.
- Audio goes directly from the iPhone to the car stereo over AUX. It does not pass through the PSP or the PSP-ICE Wi-Fi stream.

## 1. iPhone companion app

The companion app is the primary host for the end-user system.

Responsibilities:
- Render a dedicated dashboard designed for 480×272 pixels.
- Manage the PSP session, pairing, connection status, and reconnect attempts.
- Send UI frames or other suitable display updates to the PSP.
- Receive PSP button/analog events and map them to interface and media actions.
- Integrate with Spotify for playback control and now-playing information, subject to the current Spotify SDK/API, account, authentication, and iOS requirements.
- Keep music playback on the iPhone so the iPhone can output it directly to the car AUX input.

The companion app should not simply mirror the iPhone screen. It owns a purpose-built infotainment interface.

### iOS networking constraints to validate

The user enables Personal Hotspot in iOS settings, then connects the PSP to it. Do not assume an ordinary third-party iOS app can silently enable Personal Hotspot.

The app will need the appropriate iOS local-network permission and must be tested with the PSP as a hotspot client. UDP discovery/broadcast behaviour, hotspot routing, app lifecycle, and reconnect behaviour must be validated on real iPhones. If automatic discovery is unreliable, provide a simple fallback such as manual pairing or entering the PSP address.

## 2. PSP client

Target hardware: Sony PSP-2004 running custom firmware (ARK-4 / ARK-5).

Responsibilities:
- Join the iPhone Personal Hotspot over Wi-Fi.
- Discover/pair with the companion app or connect using a fallback address.
- Receive UI/video data and display it at the PSP's native 480×272 resolution.
- Send button and analog-stick events to the phone.
- Show a clear disconnected/reconnecting state.
- Remain lightweight and responsive.

The project targets the Wi-Fi connectivity available under ARK-4 and ARK-5 with iPhone hotspots. Connection stability and throughput still need to be measured on the actual target PSP and iPhone.

## 3. Wi-Fi transport and protocol

The existing PSP-ICE UDP protocol is the starting point for the direct phone-to-PSP link.

Keep transport responsibilities separated:
- **Session/control:** hello, pairing, capability negotiation, ping/pong, disconnect, reconnect.
- **Display:** UI/video frames or display updates from phone to PSP.
- **Input:** button/analog events from PSP to phone.

The current protocol header and input-state definitions are in `psp/include/protocol.h`. The existing protocol already defines control, video, input, and audio channel identifiers; the selected product architecture does not require an audio channel for the initial AUX setup.

Design for the PSP's legacy Wi-Fi throughput:
- Never assume modern Wi-Fi bandwidth.
- Prefer compact updates and avoid sending unchanged screen content unnecessarily.
- Measure real throughput, latency, packet loss, and frame rate before selecting a codec, resolution strategy, or bitrate.
- Video frames may be dropped when stale; input and session events need prompt, robust handling.
- Consider a static/low-motion UI first, since the initial product is music-focused.

## 4. Audio architecture — selected baseline: phone-direct AUX

The selected initial audio route is **Option B: the iPhone is the audio source and connects directly to the car's AUX input**.

```text
Spotify / music playback on iPhone
          │
          ├── Wi-Fi ──→ PSP
          │             (display and input only)
          │
          └── iPhone audio output
                 │
                 └── Lightning-to-3.5 mm adapter
                        │
                        └── AUX cable → car stereo
```

For an iPhone 13, a compatible Lightning-to-3.5 mm audio adapter and a 3.5 mm AUX cable are required. The PSP is not used as an audio DAC and does not need to receive or decode audio.

Bluetooth audio may be explored later as an optional alternative for cars with Bluetooth audio support, but AUX is the selected baseline and should be the first documented/tested setup.

## 5. UI and interaction design

### Visual direction

A purpose-built automotive interface combining:
- BMW iDrive-inspired layout, dark surfaces, elegant information panels, and restrained visual hierarchy.
- CarPlay-inspired app tiles and glanceable music controls.
- PSP-specific layout and physical-button interaction.

Do not copy proprietary BMW or Apple assets or imply official affiliation.

### Display constraints

- Native PSP resolution: 480×272.
- Design at the target resolution from the beginning.
- Use large typography, high contrast, simple icons, and generous focus states.
- Avoid dense layouts and tiny touch targets; the PSP is controlled with physical buttons.

### Initial screens

**Home dashboard**
- Music / Spotify
- Now Playing shortcut
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

## 6. Optional Linux development gateway

The existing `gateway/` code remains valuable for:
- Testing UDP protocol changes without the iPhone app.
- Sending test frames and input events.
- Inspecting packets and measuring performance.
- Developing the PSP client independently of iOS.
- Comparing behaviour between a PC host and the eventual iPhone host.

It is a development tool, not a required component of the intended in-car installation.

## 7. Development phases

1. Keep the existing PC gateway ↔ PSP network prototype working.
2. Build a static 480×272 dashboard mock-up and validate PSP rendering performance.
3. Implement PSP button/input transport and robust session handling.
4. Build the iPhone companion app shell and local-network permissions.
5. Connect iPhone hotspot ↔ PSP and implement discovery/pairing/reconnect.
6. Stream the phone-rendered interface to the PSP.
7. Integrate Spotify playback controls and now-playing metadata within current platform/API limits.
8. Test iPhone Lightning audio adapter → AUX → car stereo.
9. Refine automotive UI, mounting, and usability.

## Design principles

- Phone is the host; PSP is the display/controller terminal.
- No PC is required for normal end-user operation.
- Dedicated music-first UI, not phone-screen mirroring.
- AUX audio comes directly from the iPhone.
- Keep the PSP client lightweight.
- Separate display, input, session, and audio responsibilities.
- Be realistic about PSP Wi-Fi bandwidth and iOS background/network restrictions.
- Keep the Linux gateway as an optional development tool.
