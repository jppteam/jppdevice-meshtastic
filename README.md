![Banner](docs/banner.webp)

# Meshtastic for J++Device

Meshtastic is an open source, off-grid, decentralized mesh network designed for LoRA. J++Device is a small handheld device based on ESP32-C6 which has, among other things, an SX1276 LoRa radio module.

This is a fork of [meshtastic/firmware](https://github.com/meshtastic/firmware).
The port lives on the **`jppdevice` branch**, on top of upstream tag
`v2.7.24.472b14c`. It is a private board port, not a proposal for mainline.

## Installation

Just download the latest firmware binary from the [Releases page](https://github.com/jppteam/jppdevice-meshtastic/releases) and install it using either [J++Device Manager](https://jppdevice.by.m4l3vi.ch/manager) or `esptool`.

## Building and development

> [!WARNING]
>
> This port is completely vibecoded by Claude Opus 5. Keep this in mind if you ever want to work on this repo.

- **[docs/jppdevice-build-guide.md](docs/jppdevice-build-guide.md)** — build, flash, configure, update
- **[docs/jppdevice-hardware.md](docs/jppdevice-hardware.md)** — hardware spec: pins, buses, measured ADC bands

```bash
pio run -e jppdevice            # build
pio run -e jppdevice -t upload  # flash
```

Everything board-specific is under `variants/esp32c6/jppdevice/`,
`src/gps/DS1307.*`, `src/input/JppDeviceKeyboard.*` and `docs/jppdevice-*.md`.
See `git log --oneline v2.7.24.472b14c..jppdevice` for the full change set.
