# PSP-ICE development tools

## PC ↔ PC protocol test

The first network milestone uses two normal PCs/processes and UDP. No PSP or CarPlay hardware is required.

### 1. Sync the repository

```bash
git pull --rebase
```

### 2. Start the gateway

On PC A:

```bash
python3 gateway/src/server.py --host 0.0.0.0 --port 39000
```

### 3. Run the client

On PC B, replace `PC_A_IP` with the LAN address of PC A:

```bash
python3 tools/test_client.py PC_A_IP --port 39000
```

For a same-PC loopback test:

```bash
python3 gateway/src/server.py
python3 tools/test_client.py 127.0.0.1
```

Expected client output includes a successful HELLO/PONG followed by five PING/PONG exchanges and RTT measurements.

### 4. Run protocol self-tests

```bash
python3 tools/test_protocol.py
```

Expected:

```text
PSP-ICE protocol tests: PASS
```

## Protocol header

The v1 header is 14 bytes, network byte order:

```text
magic[4] version[1] channel[1] type[1] flags[1] sequence[4] payload_length[2]
```

The current implementation uses UDP because video can later discard stale packets while control/input can be handled separately.
