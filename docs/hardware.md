# PSP-ICE Hardware Notes

## Scope and current target

This page records the hardware assumptions for the current prototype. It is not a list of fully validated compatible devices.

### PSP

Initial target:
- Sony PSP-2004 (PSP-2000 family)
- Custom firmware: ARK-4 / ARK-5
- Native display: 480×272
- Built-in Wi-Fi for the local connection
- Physical buttons and analog nub as the primary controller
- 3.5 mm headphone output used as the audio output to the car

The current client is built as PSP homebrew. The documented build environment is Linux with PSPDEV/PSPSDK.

### Phone and network host

The intended end-user host is an **iPhone** running a future PSP-ICE companion app. The iPhone is intended to provide application logic, media metadata/controls, the audio source and the Personal Hotspot network.

The current development test phone is an iPhone 13. This records the test setup, not a claim that every iPhone model or iOS version has been validated.

Android is not currently documented as a supported companion-app target. The PSP client is a network endpoint and does not, by itself, establish Android compatibility.

## Current tested network arrangement

The recorded audio test used:
1. An iPhone Personal Hotspot as the Wi-Fi access point.
2. A MacBook connected to that hotspot running the Python PCM test sender.
3. A PSP connected to the same hotspot receiving and playing the PCM stream.

In this test, the iPhone provided the network only. The MacBook was the audio sender; the iPhone was not yet sending audio.

The project has not completed a compatibility matrix across iPhone models, iOS versions, hotspot configurations or other access points. The PSP's current outbound control/input path uses limited IPv4 broadcast, which may behave differently across networks. See [Protocol](protocol.md).

## Audio connection to the car

Required baseline:
- PSP 3.5 mm headphone output
- 3.5 mm AUX cable
- Car stereo with AUX input

Audio path:

```text
iPhone audio source (future sender)
       │
   local Wi-Fi / UDP
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

The PSP must remain in the audio path: it receives digital audio over Wi-Fi, plays it through its own audio hardware, and outputs analog audio through its headphone jack.

The PSP's output level should be set carefully. Start tests at low volume and increase gradually. Avoid max-volume testing.

## Development equipment

A PC/Linux host or MacBook is useful for:
- Building the PSP homebrew client (the canonical documented build environment is Linux with PSPDEV).
- Running the optional gateway and PCM test sender.
- Sending test packets and inspecting protocol traffic.
- Debugging UDP transport and measuring performance.

A PC or Raspberry Pi is not part of the intended end-user installation.

## Physical installation considerations (future)

These are design considerations, not completed installation features:

- Securely mount the PSP; do not allow it to become a projectile during sudden braking.
- Keep the display readable without obstructing the driver's view.
- Route AUX, USB/power and other cables safely, away from pedals, steering, airbags and moving controls.
- Consider PSP power/charging and heat management for longer sessions.
- Keep the system usable without interacting with the phone while driving. Configure and pair while parked.

## Compatibility status

| Component | Current status |
|---|---|
| PSP-2004 | Initial development target; client/audio prototype tested |
| Other PSP models | Not established by current project tests |
| iPhone 13 | Used as Personal Hotspot in the recorded network/audio test |
| Other iPhone/iOS combinations | Not yet documented as tested |
| Android companion app | Not implemented or established as supported |
| Car AUX input | Required baseline; vehicle installation testing remains future work |

For build instructions and reproducible test steps, see [Development Setup](development-setup.md).
