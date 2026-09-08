# PSP-ICE Architecture

## High-level design

```text
iPhone
  │
  │ CarPlay
  ▼
Linux CarPlay Gateway
  │
  │ PSP-ICE transport over Wi-Fi
  ▼
Sony PSP-2004
  │
  ├── Display
  ├── Buttons / analog input
  └── Audio → AUX → car stereo
```

The first development target is the PC gateway. A Raspberry Pi or other small Linux computer can become the eventual in-car gateway.

## Components

### Gateway

Responsible for:

- CarPlay connection/integration
- Video conversion and transport
- Audio handling
- Input handling
- PSP session management
- Diagnostics

### PSP client

Responsible for:

- Wi-Fi connection
- Receiving video frames
- Displaying the UI at PSP resolution (480×272)
- Sending buttons/analog input to the gateway
- Local playback/volume handling where appropriate

### Transport

The PSP link should be designed for low latency and unreliable Wi-Fi. Video and input are therefore treated separately: stale video data may be dropped, while input events should be delivered reliably and quickly.

## Development phases

1. PC gateway ↔ PC test client
2. PC gateway ↔ PSP network test
3. PSP input transport
4. PSP video receiver
5. Audio path
6. CarPlay integration
7. Automotive hardware integration
