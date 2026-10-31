# PSP-ICE Hardware Notes

## Target PSP

Initial target:
- Sony PSP-2004 (PSP-2000 family)
- Custom firmware: ARK-4 / ARK-5
- Native display: 480×272
- Built-in Wi-Fi for the local connection to the iPhone Personal Hotspot
- Physical buttons and analog nub as the primary controller
- 3.5 mm headphone output used as the audio output to the car

The PSP must remain in the audio path: it receives digital audio over Wi-Fi, plays it through its own audio hardware, and outputs analog audio through its headphone jack.

## Phone

Initial phone target: iPhone (current development test phone: iPhone 13).

The iPhone is intended to run the PSP-ICE companion app, provide the audio source, metadata and controls, and host the local Wi-Fi network through Personal Hotspot. The user enables Personal Hotspot through iOS settings; do not assume the app can silently enable it.

For the current network test, both the PSP and MacBook joined the iPhone Personal Hotspot. The MacBook ran the Python PCM test sender; the PSP received and played the audio. The iPhone was the access point, not yet the audio sender.

## Audio connection to the car

Required baseline:
- PSP 3.5 mm headphone output
- 3.5 mm AUX cable
- Car stereo with AUX input

Audio path:

```text
iPhone audio source
       │
   local Wi-Fi
       │
      PSP
 (audio playback)
       │
  3.5 mm output
       │
   AUX cable
       │
  Car stereo
```

The PSP's output level should be set carefully. Start tests at low volume and increase gradually. Avoid max-volume testing.

The iPhone does **not** connect directly to the car AUX input in the selected architecture. The PSP is the audio endpoint and its headphone output feeds the car.

## Development equipment

A PC/Linux host or MacBook remains useful for:
- Building the PSP homebrew client (the canonical PSPDEV environment is currently Linux).
- Running the optional gateway and PCM test sender.
- Sending test packets and frames.
- Debugging UDP transport and measuring performance.

A PC or Raspberry Pi is not part of the intended end-user installation.

## Physical installation considerations (future)

- Securely mount the PSP; do not allow it to become a projectile during sudden braking.
- Keep the display readable without obstructing the driver's view.
- Route AUX, USB/power and other cables safely, away from pedals, steering, airbags and moving controls.
- Consider PSP power/charging and heat management for longer sessions.
- Keep the system usable without interacting with the phone while driving. Configure and pair while parked.
