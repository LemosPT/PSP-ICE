# PSP-ICE

**PSP In-Car Entertainment** — a CarPlay-style in-car entertainment system using a Sony PSP as the display and controller.

## Concept

```text
iPhone → CarPlay gateway → PSP-ICE gateway → Wi-Fi → PSP
                                      ↓
                                     AUX
                                      ↓
                                  car stereo
```

The PSP is intended to act as the in-car display/input terminal. A Linux gateway handles the heavy networking and CarPlay integration.

## Target hardware

- Sony PSP-2004
- PSP custom firmware / ARK-4
- Linux gateway (development initially on PC)
- Wi-Fi
- Car AUX input

## Project status

- [ ] PSP ↔ gateway networking
- [ ] PSP video receiver
- [ ] PSP input transport
- [ ] Audio output
- [ ] CarPlay gateway integration
- [ ] Mercedes integration
- [ ] Automotive enclosure / mounting

## Repository layout

- `psp/` — PSP homebrew client
- `gateway/` — Linux gateway
- `protocol/` — PSP ↔ gateway protocol
- `docs/` — architecture and hardware documentation
- `tools/` — development/testing utilities
- `hardware/` — car-side hardware notes and schematics

## Status

Early development / architecture phase.
