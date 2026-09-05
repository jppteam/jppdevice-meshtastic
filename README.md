![Banner](docs/banner.webp)

# Meshtastic for J++Device

Meshtastic is an open source, off-grid, decentralized mesh network designed for LoRA. J++Device is a small handheld device based on ESP32-C6 which has, among other things, an SX1276 LoRa radio module.

This is a fork of [meshtastic/firmware](https://github.com/meshtastic/firmware).
The port lives on the **`jppdevice` branch**, on top of upstream tag
`v2.7.24.472b14c`. It is a private board port, not a proposal for mainline.

## Installation

Just download the latest firmware binary from the [Releases page](https://github.com/jppteam/jppdevice-meshtastic/releases) and install it using either [J++Device Manager](https://jppdevice.by.m4l3vi.ch/manager) or `esptool`.

## First-time setup

If you've never used Meshtastic before, this is the shortest path from a
freshly flashed J++Device to sending your first message. You control the node
from a phone app; the node itself is the radio.

### 1. Install the app

- **Android** — [Meshtastic on Google Play](https://play.google.com/store/apps/details?id=com.geeksville.mesh) (or the APK from [meshtastic.org/download](https://meshtastic.org/download/))
- **iOS** — [Meshtastic on the App Store](https://apps.apple.com/us/app/meshtastic/id1586432531)

### 2. Bluetooth or Wi-Fi — which to pick

The ESP32-C6 can run **either** Bluetooth **or** Wi-Fi for the phone
connection, not both at once. Meshtastic ships with Bluetooth enabled.

| | Bluetooth (default) | Wi-Fi |
|---|---|---|
| Range to phone | in the room with you | anywhere on the same LAN |
| Setup effort | pair and go | must enter SSID/password on the node first |
| Best for | carrying the node around, field use | a fixed node at home you want to reach from your computer too |
| Power | lower | higher |

**Start with Bluetooth.** Switch to Wi-Fi only if you want a stationary node
reachable over your network (it also enables the node's web UI). You set Wi-Fi
credentials from the app once you're already connected over Bluetooth, under
*Radio Configuration → Network*; the node reboots onto Wi-Fi and Bluetooth
goes quiet.

### 3. Pair the node

1. Power on the J++Device. The OLED shows a Meshtastic screen with the node
   name and a Bluetooth status line.
2. Open the app → **Add device** / the connection icon → pick your node from
   the list. It appears as `Meshtastic_xxxx`.
3. When prompted for a PIN, read the 6-digit code shown on the J++Device OLED
   and type it into the phone. (If no PIN appears, the default is `123456`.)
4. The app switches to the channel/node view once connected.

### 4. Set region (required)

The first time the app connects it asks for your **region** (e.g. `EU_868`,
`US`). The radio will not transmit until this is set. Pick the region you are
physically in. For Russia, use the `RU` region.

> The J++Device RF front-end is **868 MHz only** — if you are in a region
> that would otherwise use 915 MHz, this hardware still can't legally or
> effectively use it.

### 5. Send a message

- **Primary channel** — out of the box the node joins the public `LongFast`
  channel. Anyone nearby on default settings is on it too. Type in the
  channel view to broadcast.
- **Your identity** — set your long and short name under *Radio Configuration
  → User*. The short name shows on other nodes' screens.
- **Check the mesh** — the *Nodes* tab lists every node you've heard, with
  last-heard time and signal. An empty list just means nobody else is in
  range yet; the node is still listening.

### 6. Keep it powered

The node relays for the mesh only while it's on. For a home node, leave it on
USB power. For carrying, a charged battery — the OLED shows the battery
percentage (see the battery-monitor divider in the hardware spec if the
reading looks off).

To go further — private channels, MQTT uplink, position sharing, module
settings — see the [official Meshtastic docs](https://meshtastic.org/docs/).

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
