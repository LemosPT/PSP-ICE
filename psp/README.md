# PSP Client

The PSP client is the in-car display, physical controller and audio playback endpoint for PSP-ICE.

## Build requirements

The project is built for the PlayStation Portable using the PSP SDK and included Makefile.

Requirements:
- PSP SDK / pspsdk installed.
- `psp-gcc`, `make` and the PSP toolchain available in `PATH`.

The current canonical build environment is Linux with PSPDEV installed at `/usr/local/pspdev`. Set:

```bash
export PSPDEV=/usr/local/pspdev
export PSPSDK=$PSPDEV/psp/sdk
export PATH="$PSPDEV/bin:$PATH"
```

From the PSP project directory:

```bash
make clean
make
```

The build produces `EBOOT.PBP`. Copy it to:

```text
PSP/GAME/<your_game_name>/EBOOT.PBP
```

Then launch it from the PSP home screen.

## Current client functionality

- Connects to Wi-Fi using the PSP network APIs.
- Opens and binds a UDP socket on port 39000.
- Sends PSP controller state over the PSP-ICE input channel.
- Receives AUDIO_DATA UDP packets.
- Validates the expected 1,024-byte PCM payload.
- Queues audio for a separate playback thread.
- Plays 44.1 kHz, stereo, signed 16-bit PCM through the PSP audio hardware.

The current test receiver uses a 16-packet queue and a 4-packet prebuffer. These values are prototype settings and may change after diagnostics and tuning.

## Audio output

The PSP is intentionally part of the audio path:

```text
Android/iPhone → Wi-Fi → PSP audio playback → 3.5 mm headphone output → AUX → car stereo
```

The iPhone-originated sender is not implemented yet. For development, use the Python test sender in `gateway/src/audio_sender.py`.

## Troubleshooting

- If the PSP cannot connect, verify that it and the host are on the same local network/hotspot.
- If no audio is heard, verify the PSP IP, UDP port 39000, host firewall and sender destination.
- Start with low PSP and car volume, then increase gradually.
- If audio crackles, first test both devices close to the access point. If crackling persists, investigate packet loss, audio queue underruns and queue synchronization.
- If `make` cannot find the compiler, verify `PSPDEV`, `PSPSDK` and `PATH`.

[PSP-ICE | Official Community ❄️](https://discord.gg/BaQJ3Zvm2V)
