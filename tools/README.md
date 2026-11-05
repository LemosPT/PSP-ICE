# PSP-ICE development tools

This directory contains small utilities for testing the current protocol implementation.

For a complete setup from a fresh clone—including Python tests, PC-to-PC UDP tests, PSP builds and the PCM audio test—see the [Development Setup guide](../docs/development-setup.md).

## Available tools

| Tool | Purpose |
|---|---|
| `test_protocol.py` | Self-tests the Python v1 packet codec |
| `test_client.py` | PC-side HELLO/PING/PONG client for the development gateway |
| `test_stream.py` | Reserved/empty placeholder; no stream test is implemented yet |

The PCM test sender lives in `gateway/src/audio_sender.py`, and the UDP development server is `gateway/src/server.py`.

## Run the protocol self-test

From the repository root:

```bash
python3 tools/test_protocol.py
```

Expected output:

```text
PSP-ICE protocol tests: PASS
```

This checks the Python packet codec only. It does not test PSP hardware, Wi-Fi or audio playback.

## PC ↔ PC protocol test

Start the gateway on PC A:

```bash
python3 gateway/src/server.py --host 0.0.0.0 --port 39000
```

On PC B, replace `PC_A_IP` with PC A's LAN address:

```bash
python3 tools/test_client.py PC_A_IP --port 39000
```

For a same-PC loopback test, run the server in one terminal and this in another:

```bash
python3 tools/test_client.py 127.0.0.1
```

Expected client output includes a successful HELLO/PONG followed by five PING/PONG exchanges and RTT measurements.

## Protocol header

The v1 header is 14 bytes, network byte order:

```text
magic[4] version[1] channel[1] type[1] flags[1] sequence[4] payload_length[2]
```

For channel values, payload layouts, known implementation gaps and the current PSP/Python button-bit mismatch, see [Protocol Notes](../docs/protocol.md).

## Scope

These scripts are development/test utilities. They do not provide the planned iPhone companion app, music-service integration, production pairing/discovery, or end-to-end video/UI streaming.
