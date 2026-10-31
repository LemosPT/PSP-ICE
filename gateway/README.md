# Development Gateway

The gateway is an optional PC/Linux/macOS development tool. It is not part of the intended final in-car installation.

It currently provides:
- A UDP test server for receiving PSP control/input messages.
- A Python PCM test sender for sending audio to the PSP.
- A shared Python implementation of the PSP-ICE v1 packet format.

## Requirements

- Python 3.10+ (the scripts use standard-library modules only).
- Mac/Linux/Windows host with Python and local network access to the PSP.
- PSP connected to the same LAN or phone hotspot as the host.

## Run the UDP server

From the repository root:

```bash
cd gateway/src
python3 server.py
```

The server listens on UDP port 39000 by default. If using UFW on Linux:

```bash
sudo ufw allow 39000/udp
```

## Send test audio

First connect the PSP and the computer running the sender to the same local network. For the current hotspot test, both devices joined the iPhone Personal Hotspot.

Find the PSP's IP address in its network connection details, then run:

```bash
cd gateway/src
python3 -m py_compile audio_sender.py protocol.py
python3 audio_sender.py <PSP_IP>
```

Example:

```bash
python3 audio_sender.py 172.20.10.3
```

The sender generates a 440 Hz stereo test tone for 15 seconds by default. Options:

```bash
python3 audio_sender.py --help
python3 audio_sender.py <PSP_IP> --frequency 440 --amplitude 5000 --seconds 15
```

Start with PSP/car volume low and increase gradually.

## Current audio test format

- 44,100 Hz
- Stereo
- Signed 16-bit little-endian PCM
- 256 frames per packet
- 1,024 PCM bytes plus 14-byte PSP-ICE header
- UDP destination port 39000

The PSP receiver buffers and plays the incoming packets through its own audio output. Connect the PSP's 3.5 mm headphone output to the car stereo AUX input to test the intended physical audio path.

## Known limitations

- `audio_sender.py` is a synthetic test-tone generator, not an iPhone audio sender or music-service integration.
- The sender currently targets a manually supplied PSP IP; discovery/pairing is not implemented.
- The receiver's audio sequence-loss accounting, queue underrun/overflow counters and concurrency hardening remain future work.
- The video channel is not yet a completed end-to-end UI streaming implementation.

## Development note

The iPhone Personal Hotspot has been tested as the local network with both the PSP and MacBook connected. Clean PCM playback was confirmed with the MacBook running this sender. The iPhone companion app and its audio sender remain future work.
