# PSP-ICE Development Roadmap

This roadmap separates demonstrated prototype work from intended product work. It is a planning document, not a promise that every item is scheduled.

## Current milestone

**Early prototype — local dashboard scaffold and audio diagnostics are implemented in source; end-user phone integration remains future work.**

Demonstrated or implemented in the current source:
- PSP UDP networking and controller input sent to a development host.
- Python PC-side gateway and protocol test client.
- Python PCM test sender streaming a 440 Hz tone to the PSP.
- Buffered 44.1 kHz stereo PCM playback through the PSP headphone output.
- iPhone Personal Hotspot used as the access point while a MacBook sends audio to the PSP.
- Local PSP GU dashboard scaffold with six selectable tiles, D-pad navigation, Cross to open and Circle to return.
- Audio sequence-gap, duplicate and out-of-order counters in the PSP main loop.
- Estimated packet-loss percentage rendered on the dashboard.
- Audio queue mutex protection and received/played/dropped/playback-error/queue-depth statistics exposed by the audio module.

The dashboard is a local UI scaffold, not the completed phone-driven infotainment interface. Its app pages are placeholders; the Wi-Fi/Bluetooth indicators are decorative, and the media strip is static. The dashboard has not been independently verified here on physical PSP hardware.

The iPhone is not yet the audio sender. The companion app and end-to-end PSP display transport are not implemented.

## Prioritized next phases

### P0 — Stabilize the existing prototype

- [ ] Validate the canonical button-bit mapping with PSP hardware and gateway diagnostics.
- [ ] Add audio underrun detection and expose useful queue/playback statistics in diagnostics.
- [ ] Decide whether duplicate/out-of-order packets should be discarded or reordered before queueing.
- [ ] Test packet loss, jitter and longer playback sessions on physical PSP hardware.
- [ ] Document repeatable PSP and hotspot test results.
- [ ] Add automated CI for Python protocol tests.

### P1 — Make the phone-to-PSP connection viable

- [ ] Define a connection lifecycle: startup, HELLO, peer selection, timeout, disconnect and reconnect.
- [ ] Replace or supplement limited-broadcast assumptions with a robust pairing/manual-address path.
- [ ] Validate iOS local-network permission and Personal Hotspot behaviour on target devices.
- [ ] Define capability/version negotiation before companion-app integration.

### P2 — Implement the iPhone host

- [ ] Create the native iPhone companion-app project.
- [ ] Implement session/network handling for protocol v1.
- [ ] Select a technically and legally viable audio source path.
- [ ] Implement the iPhone-originated PCM sender.
- [ ] Add metadata and playback controls within provider/platform API limits.

### P3 — Build the PSP infotainment experience

- [ ] Implement the PSP-native 480×272 dashboard and Now Playing screen.
- [ ] Define and implement a display-update format.
- [ ] Benchmark throughput, frame size, latency and memory use on PSP hardware.
- [ ] Integrate PSP input with the companion app.

### P4 — Vehicle integration and release readiness

- [ ] Test PSP headphone output → AUX → car stereo in the vehicle.
- [ ] Design a secure mount and safe cable/power routing.
- [ ] Test heat, charging and longer sessions.
- [ ] Document installation, setup, recovery and known limitations.
- [ ] Prepare screenshots/demo and a user manual.

## Out of scope for the first usable milestone

Navigation, phone calls, broad multi-provider support and deeper vehicle integration are later possibilities. They should not block a reliable music-first prototype.

## Status terminology

- **Done / tested:** verified in the current project setup.
- **In progress:** implementation has started but is not complete.
- **Planned:** intended future work.
- **Unverified:** not enough evidence to claim support.
