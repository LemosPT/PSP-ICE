# Documentation

PSP-ICE documentation is organized by topic:

| Page | What it covers |
|---|---|
| [Architecture](architecture.md) | System roles, data flow, audio path and development phases |
| [Hardware](hardware.md) | PSP/phone targets, car audio connection and installation considerations |
| [Protocol](protocol.md) | Current UDP packet format, channels, payloads and known implementation gaps |
| [Development setup](development-setup.md) | Prepare a host, run protocol tests, build the PSP client and test audio |

## Project status at a glance

PSP-ICE is an early prototype. The current demonstrated work is the PSP UDP client, controller-state transmission, Python development gateway and PCM test-audio playback through the PSP headphone output.

The iPhone companion app, iPhone-originated audio sender, production pairing/reconnect flow and PSP infotainment UI/display transport are not yet implemented. Read the individual pages for the distinction between implemented behaviour, tested prototype settings and proposals.

## Documentation conventions

- **Implemented** means code for the feature exists in the repository.
- **Tested** means the project has recorded a successful test on actual hardware or a reproducible development setup.
- **Proposed** means a design has been documented but is not an implemented feature.
- **Planned** means future work, not a current capability.

When code and documentation disagree, treat the code as evidence of current behaviour and record the discrepancy rather than assuming the intended design is already implemented.
