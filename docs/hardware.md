# PSP-ICE Hardware Notes

## Target PSP

Initial target:
- Sony PSP-2004 (PSP-2000 family)
- Custom firmware: ARK-4 / ARK-5
- Native display: 480×272
- Built-in Wi-Fi used for the local connection to the iPhone Personal Hotspot
- Physical buttons and analog nub used as the primary controller

The project targets the existing Wi-Fi compatibility of ARK-4/ARK-5 with iPhone hotspots. This is a project test assumption based on the target setup; connection reliability, hotspot band/security compatibility, throughput, and reconnect behaviour must still be validated on the actual PSP/iPhone pair.

## Phone

Initial phone target: iPhone.

The iPhone runs the PSP-ICE companion app and music playback. It creates/enables Personal Hotspot through iOS settings; the app should guide the user rather than assume it can silently turn the hotspot on.

For the selected audio setup, an iPhone without a 3.5 mm headphone jack (including iPhone 13) needs:
- A compatible Lightning-to-3.5 mm audio adapter
- A 3.5 mm AUX cable
- A car stereo with an AUX input

Audio path:
```text
iPhone → Lightning-to-3.5 mm adapter → AUX cable → car stereo
```

The PSP is not in the audio path.

## Car stereo

Baseline requirement:
- AUX input

Bluetooth audio may be supported as an optional future alternative, but is not required for the first version.

## Development equipment

A PC/Linux host remains useful for:
- Building the PSP homebrew client
- Running the existing gateway/test host
- Sending test packets and frames
- Debugging UDP transport and measuring performance

A PC or Raspberry Pi is not part of the intended end-user installation.

## Physical installation considerations (future)

- Securely mount the PSP; do not allow it to become a projectile during sudden braking.
- Keep the display readable without obstructing the driver's view.
- Route AUX and charging cables safely, away from pedals, steering, airbags, and moving controls.
- Consider PSP power/charging and heat management for longer sessions.
- Keep the system usable without interacting with the phone while driving. Configure and pair while parked.
