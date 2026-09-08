# PSP-ICE

CarPlay-style in-car entertainment system using a Sony PSP as the display and controller.

## Project status

- [ ] PSP ↔ gateway networking
- [ ] PSP video receiver
- [ ] PSP input transport
- [ ] Audio output
- [ ] CarPlay gateway integration
- [ ] Mercedes integration

## Target hardware

- Sony PSP-2004
- PSP custom firmware / ARK-4
- Linux gateway (initial development on PC)
- Car AUX input

## Architecture

```
iPhone → CarPlay gateway → PSP-ICE gateway → Wi-Fi → PSP
                                      ↓
                                     AUX
                                      ↓
                              Mercedes stereo
```
