# PSP-ICE Architecture

## High-level design

PSP-ICE uses the Sony PSP as the in-car display and physical input device, while a Linux gateway handles the heavier networking, CarPlay integration, media processing, and routing.

```text
iPhone
  │
  │ CarPlay
  ▼
Linux CarPlay Gateway
  │
  │ PSP-ICE transport over Wi-Fi
  ▼
Sony PSP-2004
  │
  ├── Display (480×272)
  ├── Buttons / analog input
  └── Optional audio → AUX → car stereo

Audio can alternatively bypass the PSP:

iPhone ───── Bluetooth ─────→ Car stereo
                                  │
                                  ├── Audio
                                  ├── Hands-free calls (if supported)
                                  └── AVRCP / steering-wheel controls (if supported)
```

The first development target is the PC gateway. A Raspberry Pi or other small Linux computer can become the eventual in-car gateway.

## Components

### Gateway

Responsible for:

- CarPlay connection/integration
- Video conversion and transport
- Audio routing and audio-mode management
- Input handling
- PSP session management
- Diagnostics
- Coordinating phone/car connectivity where required

The gateway should not assume that audio has to pass through the PSP. Audio is an independent subsystem so that different car configurations can be supported.

### PSP client

Responsible for:

- Wi-Fi connection
- Receiving video frames
- Displaying the UI at PSP resolution (480×272)
- Sending buttons/analog input to the gateway
- Optional local audio output when AUX mode is selected
- Local volume handling where appropriate

### Transport

The PSP link should be designed for low latency and unreliable Wi-Fi. Video and input are therefore treated separately: stale video data may be dropped, while input events should be delivered reliably and quickly.

## Audio architecture

PSP-ICE supports multiple possible audio paths. The selected mode should be configurable rather than hard-coded into the PSP client.

### AUX mode

In AUX mode, audio is sent to the PSP and then physically connected to the car stereo through the PSP's audio output.

```text
iPhone
  │
  ▼
CarPlay Gateway
  │
  ├── Video/Input ──→ Wi-Fi ──→ PSP
  │                              │
  │                              └── Audio ──→ AUX ──→ Car stereo
  │
  └── CarPlay/media processing
```

Advantages:

- Works with cars that only provide an AUX input
- No requirement for Bluetooth audio support in the car
- Simple and predictable physical audio path

### Bluetooth car mode

In Bluetooth mode, the phone's audio does not need to be passed through the PSP. The phone connects directly to the car stereo using its normal Bluetooth audio/call connection.

```text
iPhone
  │
  ├── CarPlay → PSP-ICE Gateway → Wi-Fi → PSP
  │                                      │
  │                                      └── Display/Input
  │
  └── Bluetooth ───────────────────────→ Car stereo
                                         │
                                         ├── Music/audio
                                         ├── Hands-free calls*
                                         └── AVRCP / steering controls*
```

This makes the PSP primarily a display/controller terminal while the car stereo remains responsible for audio playback and, where supported, hands-free calling and Bluetooth media controls.

`*` Availability depends on the specific phone and car stereo. PSP-ICE should not assume that every Bluetooth head unit exposes hands-free or steering-wheel functionality.

### Future audio modes

The architecture should allow additional backends later, for example:

- USB audio
- External USB/Bluetooth DAC
- Direct gateway audio output
- Other vehicle-specific audio interfaces

## Audio mode selection

A future PSP-ICE settings interface should expose an audio output selection such as:

```text
Audio Output

● AUX / PSP
○ Bluetooth → Car
○ Automatic
```

`Automatic` may select the appropriate route based on the detected/known car configuration, but explicit selection should always be available.

## Input and vehicle controls

PSP buttons and analog input are transported from the PSP to the gateway. The gateway can then map these to CarPlay or other media controls.

Bluetooth car mode introduces a second, independent control path: the car stereo may communicate directly with the phone using Bluetooth media/call profiles. This means steering-wheel buttons can continue to work through the normal car stereo/phone connection without requiring every control event to pass through the PSP.

Where the vehicle exposes additional controls that are not handled by Bluetooth, future PSP-ICE hardware or gateway integrations may provide another input path.

## Development phases

1. PC gateway ↔ PC test client
2. PC gateway ↔ PSP network test
3. PSP input transport
4. PSP video receiver
5. Audio abstraction and AUX mode
6. Bluetooth audio/car mode investigation
7. CarPlay integration
8. Vehicle-control integration
9. Automotive hardware integration

## Design goals

- Keep the PSP client lightweight
- Keep latency low
- Separate video, input, and audio subsystems
- Support unreliable Wi-Fi gracefully
- Avoid requiring audio passthrough through the PSP when the car can handle audio directly
- Support both AUX-only and Bluetooth-capable vehicles
- Keep the gateway portable across PC, Raspberry Pi, and other Linux SBC hardware
