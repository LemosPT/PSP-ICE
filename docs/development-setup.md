# Development Setup

This guide takes a developer from a fresh checkout to the current PC-side protocol tests and PSP audio prototype. It describes the repository as it exists today; it does not set up the future iPhone companion app.

## 1. What you need

### For gateway and protocol tests

- Git
- Python 3.10 or newer
- A host running Linux, macOS or Windows
- Two devices on the same reachable local network for a two-device test (or one device for loopback)

The Python gateway and test tools currently use only the Python standard library; no third-party packages are required.

### For PSP builds and hardware tests

- A Sony PSP-2000-family device (the initial hardware target is PSP-2004)
- Custom firmware capable of running homebrew (the project targets ARK-4/ARK-5)
- A Linux development host with PSPDEV/PSPSDK installed
- A USB connection or Memory Stick reader to copy the built EBOOT to the PSP
- Wi-Fi access point or phone hotspot reachable by the PSP and development host

The final in-car installation is not intended to require the PC/Linux gateway. It is a development and test tool.

## 2. Get the source

Clone the repository and enter it:

```bash
git clone https://github.com/LemosPT/PSP-ICE.git
cd PSP-ICE
```

If you already have a checkout, update it with:

```bash
git pull --rebase
```

## 3. Run the protocol self-test

From the repository root:

```bash
python3 tools/test_protocol.py
```

A successful run currently prints:

```text
PSP-ICE protocol tests: PASS
```

This tests the Python packet codec. It does not test PSP hardware, Wi-Fi behaviour or the audio playback path.

## 4. Run a PC-to-PC UDP test

Choose one computer as the gateway/server.

On PC A:

```bash
python3 gateway/src/server.py --host 0.0.0.0 --port 39000
```

On PC B, replace `PC_A_IP` with PC A's LAN address:

```bash
python3 tools/test_client.py PC_A_IP --port 39000
```

The client sends HELLO, expects a PONG, then performs five PING/PONG round trips by default and prints the measured RTT.

For a same-computer loopback test, open two terminals. Run the server in the first:

```bash
python3 gateway/src/server.py
```

Then run the client in the second:

```bash
python3 tools/test_client.py 127.0.0.1
```

If the server receives nothing, check that both devices are on the same network, that PC A's firewall permits inbound UDP port 39000, and that the address supplied to the client is correct. On a Linux host using UFW, the corresponding rule is:

```bash
sudo ufw allow 39000/udp
```

## 5. Build the PSP client

The current documented build environment is Linux with PSPDEV installed at `/usr/local/pspdev`. Set the toolchain environment in the shell:

```bash
export PSPDEV=/usr/local/pspdev
export PSPSDK="$PSPDEV/psp/sdk"
export PATH="$PSPDEV/bin:$PATH"
```

Check that the compiler and SDK configuration tool are available:

```bash
command -v psp-gcc
psp-config --pspsdk-path
```

Build from the PSP project directory:

```bash
cd psp
make clean
make
```

The Makefile produces `EBOOT.PBP`. On the PSP Memory Stick, create a folder such as:

```text
PSP/GAME/PSP-ICE/
```

Copy `psp/EBOOT.PBP` into that folder. Safely eject the Memory Stick and launch **PSP ICE (In Car Entertainment)** from the PSP Game menu.

If `psp-gcc` or `psp-config` cannot be found, check that PSPDEV is installed and that the exported paths match its actual installation location.

## 6. Test the PSP network/input client

1. Connect the PSP to the same local network as the development host.
2. Start the gateway on the development host:

   ```bash
   python3 gateway/src/server.py --host 0.0.0.0 --port 39000
   ```

3. Launch PSP-ICE on the PSP and use the PSP network connection dialog to join the access point.
4. Watch the gateway terminal for received HELLO/input packets.

**Current implementation detail:** the PSP client currently sends its HELLO and input datagrams to `255.255.255.255:39000` (limited broadcast). Broadcast forwarding varies by network and access point. This is not a completed discovery/pairing system. If the gateway does not receive those packets, verify that the local network permits client broadcast traffic; do not assume the current code can be configured with a gateway IP from the PSP UI.

## 7. Send a test tone to the PSP

This test exercises the current audio receive/playback path. It does not stream audio from the iPhone.

1. Connect both the PSP and the computer running the sender to the same local network. The project's recorded hotspot test used the iPhone as the access point, with both the PSP and MacBook connected to it.
2. Find the PSP's current IPv4 address in its network connection details.
3. Keep PSP volume and car stereo volume low before starting.
4. From the repository root, run:

   ```bash
   python3 gateway/src/audio_sender.py PSP_IP
   ```

   For example:

   ```bash
   python3 gateway/src/audio_sender.py 192.168.1.84
   ```

The sender generates a 440 Hz stereo test tone for 15 seconds by default. Optional parameters:

```bash
python3 gateway/src/audio_sender.py --help
python3 gateway/src/audio_sender.py PSP_IP --frequency 440 --amplitude 5000 --seconds 15
```

The sender targets the manually supplied PSP IP at UDP port 39000. The PSP receives the PCM packets and plays them through its 3.5 mm headphone output. Connect that output to the car stereo AUX input if testing the complete physical audio route.

Start at low volume and increase gradually. Avoid maximum-volume testing.

## 8. Current limitations and troubleshooting

- The iPhone companion app and iPhone-originated audio sender are not implemented.
- The Python audio sender generates a synthetic test tone; it is not a music player or Spotify integration.
- PSP-side discovery, pairing, session management and reconnect behaviour are not complete.
- The PSP currently uses broadcast for its outbound control/input packets, while the Python audio sender requires the PSP's IP address.
- The PSP display/UI streaming path is not a completed end-to-end feature.
- Audio sequence-loss accounting, underrun/overflow diagnostics and queue synchronization still need hardening.
- If audio crackles, first test close to the access point and then investigate packet loss and queue behaviour.
- If the PSP cannot connect, confirm that the access point supports the PSP's Wi-Fi requirements and that the PSP can obtain an IP address.
- If the gateway receives no PSP input, check host firewall rules and broadcast behaviour on the local network.

For packet fields and implementation caveats, see [Protocol](protocol.md). For the system's intended final topology, see [Architecture](architecture.md).
