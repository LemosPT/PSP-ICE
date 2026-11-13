# PSP Client

The PSP client is the in-car display, physical controller and audio playback endpoint for PSP-ICE.

## Build requirements

The project is built for the PlayStation Portable using the PSP SDK and included Makefile.

Requirements:
- PSP SDK / PSPSDK installed.
- `psp-gcc`, `make` and the PSP toolchain available in `PATH`.

The current documented build environment is Linux with PSPDEV installed at `/usr/local/pspdev`. Set:

```bash
export PSPDEV=/usr/local/pspdev
export PSPSDK="$PSPDEV/psp/sdk"
export PATH="$PSPDEV/bin:$PATH"
```

From the repository root:

```bash
cd psp
make clean
make
```

The build produces `EBOOT.PBP`. Copy it to a folder such as:

```text
PSP/GAME/PSP-ICE/EBOOT.PBP
```

Then launch it from the PSP home screen.

For the complete host setup, PC protocol tests and hardware audio test, see [Development Setup](../docs/development-setup.md).

## Current client functionality

- Connects to Wi-Fi using the PSP network APIs.
- Opens and binds a UDP socket on port 39000.
- Sends PSP controller state over the PSP-ICE input channel.
- Sends its initial HELLO packet.
- Receives AUDIO_DATA UDP packets.
- Validates the expected 1,024-byte PCM payload.
- Queues audio for a separate playback thread.
- Plays 44.1 kHz, stereo, signed 16-bit PCM through the PSP audio hardware.

The current receiver uses a 16-packet queue and a 4-packet prebuffer. Queue access is protected by a PSP kernel semaphore mutex. The audio module exposes packet received/played/dropped counts, playback errors and queue depth. The main loop tracks sequence gaps, duplicates and out-of-order packets and displays estimated loss on the home dashboard. These are diagnostic counters; duplicate/out-of-order packets are not yet filtered or reordered, and a dedicated underrun counter is not implemented.

## Local dashboard scaffold

The current source draws a local 480×272 dashboard with six selectable tiles, D-pad navigation, Cross to open a section and Circle to return home. The Music page and other app pages are placeholders; the media strip is static. Wi-Fi/Bluetooth labels and icons are decorative rather than live radio telemetry. This is not yet the phone-driven infotainment UI, and end-to-end display transport is not implemented.

## Audio output

The PSP is intentionally part of the audio path:

```text
iPhone (intended future sender)
       │
     Wi-Fi
       │
      PSP audio playback
       │
  3.5 mm headphone output
       │
      AUX
       │
   Car stereo
```

The iPhone-originated sender is not implemented yet. For development, use the Python test sender in `gateway/src/audio_sender.py`, which generates a synthetic test tone.

The current client does not offer a completed Android/iPhone companion-app interface. The documented product target is an iPhone host.

## Current networking limitation

The PSP currently sends HELLO and controller-state datagrams to `255.255.255.255:39000`. This is limited IPv4 broadcast, not a completed discovery or pairing protocol. The Python audio test sender instead needs the PSP's IP address as an argument.

## Troubleshooting

- If the PSP cannot connect, verify that the access point supports the PSP's Wi-Fi requirements and that the PSP obtains an IP address.
- If the gateway receives no input packets, check host firewall rules and whether the local network forwards broadcast traffic.
- If no audio is heard, verify the PSP IP, UDP port 39000, host firewall and sender destination.
- Start with low PSP and car volume, then increase gradually.
- If audio crackles, first test both devices close to the access point. If crackling persists, investigate packet loss, audio queue underruns and queue synchronization.
- If `make` cannot find the compiler, verify `PSPDEV`, `PSPSDK` and `PATH`.

For packet definitions and the canonical Python/PSP button-bit mapping, see [Protocol](../docs/protocol.md).
