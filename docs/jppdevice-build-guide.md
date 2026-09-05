# J++Device — Meshtastic Build & Flash Guide

This guide covers building and flashing Meshtastic firmware for the J++Device
(ESP32-C6 + SX1276 LoRa).

**Base firmware version:** Meshtastic v2.7.24 (tag `v2.7.24.472b14c`)

---

## What this repository is

A fork of [meshtastic/firmware](https://github.com/meshtastic/firmware) carrying
the J++Device board port on the **`jppdevice` branch**, committed on top of the
upstream tag `v2.7.24.472b14c`. It is not intended to be submitted upstream.

The port is kept as a branch rather than a set of copied files so that moving to
a new Meshtastic release is a `git rebase` (see §8). Everything board-specific
lives in these paths, none of which exist upstream, so they never conflict:

| Path | Contents |
|---|---|
| `variants/esp32c6/jppdevice/` | variant header, pin map, build env, `lateInitVariant()` |
| `src/gps/DS1307.{h,cpp}` | DS1307 RTC driver |
| `src/input/JppDeviceKeyboard.{h,cpp}` | 5-key ADC ladder keypad driver |
| `docs/jppdevice-*.md` | this guide and the [hardware spec](jppdevice-hardware.md) |

The port also patches a handful of upstream files — `RTC.cpp`, `ScanI2C*`,
`InputBroker.cpp`, `Power.cpp`, `power.h` — which are where rebase conflicts
will land if any do.

```bash
git log --oneline v2.7.24.472b14c..jppdevice
```

The `esp32:` commit in that list is a general ESP32-C6 fix rather than
board-specific — see [Battery sense](#battery-readings-are-uncalibrated) below.

---

## Prerequisites

- **Python 3.9+**
- **PlatformIO CLI** — install it in its own environment:
  ```bash
  pipx install platformio
  ```
  This puts `pio` in `~/.local/bin`, which may not be on your `PATH`. Either add
  it, or prefix the commands below with `PATH="$HOME/.local/bin:$PATH"`.
- **USB-C cable** — data-capable, not charge-only
- **Git**

The first build downloads the RISC-V toolchain and the ESP32-C6 Arduino
framework (several hundred MB) and takes roughly 5–10 minutes. Later builds are
incremental and take well under a minute.

---

## 1. Prepare the tree

```bash
git clone -b jppdevice <this-repo-url> jppdevice-firmware
cd jppdevice-firmware
git submodule update --init --depth 1
```

The `protobufs` submodule is **required** — the build fails without it.

`origin` is this port; `upstream` points at `meshtastic/firmware` and is only
used to fetch new release tags (§8).

---

## 2. Build

```bash
pio run -e jppdevice
```

Expected result on a clean tree:

```
RAM:   27.0% (used 88596 bytes from 327680 bytes)
Flash: 57.7% (used 1399554 bytes from 2424832 bytes)
```

**Common errors**

- *"Unknown environment jppdevice"* — the variant's `platformio.ini` wasn't
  picked up. Verify `variants/esp32c6/jppdevice/platformio.ini` exists; the
  top-level `platformio.ini` globs it via `variants/*/*/platformio.ini`.
- *`esp32c6_base` not found* — you are on a firmware tag that predates
  `variants/esp32c6/esp32c6.ini`. Use v2.7.24 or newer.
- *Undefined reference to a driver you just added* — check that the file
  includes `configuration.h` **before** testing any variant macro. A guard like
  `#ifdef DS1307_RTC` placed above that include silently compiles the whole
  file away and only shows up as a link error.

---

## 3. Flash

```bash
pio run -e jppdevice -t upload
```

If the upload fails with "connection timeout":

1. Hold the **BOOT** button on the board
2. Press and release **RESET** while still holding BOOT
3. Release BOOT — the board is now in download mode
4. Retry the upload command

To watch the console:

```bash
pio device monitor -e jppdevice
```

The environment sets `monitor_speed = 460800` (inherited from `esp32c6_base`),
which is the rate the firmware's log output uses.

> **Note:** the board's single USB-C port has no UART bridge chip, so the
> console is the C6's native USB-Serial-JTAG (`ARDUINO_USB_CDC_ON_BOOT=1`
> plus `ARDUINO_USB_MODE=1`). No driver to install — the port shows up as
> `/dev/cu.usbmodem*`. UART0 is not reachable from a host on this board.

---

## 4. First boot

After flashing, the board resets automatically. You should see:

- The **SSD1306 OLED** showing the Meshtastic boot screen, then the main UI
- The serial console showing init messages for Screen, LoRa, SD and RTC —
  look for a `DS1307` line from the I2C scan to confirm RTC detection

**There is no WiFi access point and no web interface on first boot.** WiFi is
disabled by default (`config.network.wifi_enabled` is false until you set it),
the firmware only ever operates as a WiFi *client*, and the ESP32-C6 build
excludes the web server entirely — see [Known limitations](#7-known-limitations).

---

## 5. Configuration

Configure over **USB serial**, using the
[Meshtastic CLI](https://meshtastic.org/docs/software/python/cli/) or the
desktop/mobile app's serial connection:

```bash
meshtastic --port /dev/cu.usbmodem* --info
meshtastic --port /dev/cu.usbmodem* --set-owner "my-node"
```

Set at minimum:

- **Region** — the variant ships **EU868** as the factory default via
  `USERPREFS_CONFIG_LORA_REGION`, so the radio transmits out of the box. The
  board's RF front-end is matched for 868 MHz only; the setting is still
  writable at runtime, but moving it to another band transmits into a
  mismatched matching network. Leave it on an 868 band.
- **Owner / node name**
- **Channel** — name and encryption key

**Optional: reach the node over WiFi.** Once WiFi credentials are set, the node
joins your network as a client and exposes the Meshtastic TCP API on
**port 4403**, which the app and CLI can connect to:

```bash
meshtastic --port /dev/cu.usbmodem* --set network.wifi_enabled true
meshtastic --port /dev/cu.usbmodem* --set network.wifi_ssid "my-network"
meshtastic --port /dev/cu.usbmodem* --set network.wifi_psk "my-password"
# after it joins:
meshtastic --host <node-ip> --info
```

This is an API connection, not a web page — there is no URL to open in a
browser.

---

## 6. Keyboard controls

The 5-key resistor-ladder keypad maps to Meshtastic navigation:

| Key | Function |
|-----|----------|
| UP | Scroll up through messages / menu items |
| DOWN | Scroll down through messages / menu items |
| LEFT | Back / exit current menu |
| RIGHT | Next / enter submenu |
| SELECT | Confirm selection / send message |

The driver polls the ladder every 50 ms and requires two consecutive matching
reads before dispatching an event. If a key registers unreliably, check the
measured ADC values against the thresholds in `variant.h`
(`JPPDEVICE_KEY_*_MAX`).

---

## 7. Known limitations

**No BLE.** The ESP32-C6 base environment builds with `HAS_BLUETOOTH=0` and
`MESHTASTIC_EXCLUDE_BLUETOOTH`, and ignores the NimBLE library. Verified: the
linked image contains no NimBLE symbols. This is an upstream state, not a
hardware limit — it will work when Meshtastic enables BLE for the C6.

**No web interface.** `esp32c6_base` sets `MESHTASTIC_EXCLUDE_WEBSERVER`.
Verified: no web server symbols in the linked image. Use the CLI or app over
serial or TCP (§5).

**No GPS.** The board has no GPS module; the variant sets
`MESHTASTIC_EXCLUDE_GPS`. Time comes from the mesh or from NTP once WiFi is up,
and is persisted across power loss by the DS1307.

<a id="battery-readings-are-uncalibrated"></a>
**Battery sense takes a different code path here.** ESP-IDF's legacy
`esp_adc_cal_*` API is only declared for ESP32, S2, C3 and S3 — not the C6 — so
`Power.cpp` cannot use it. The port adds a `HAS_ESP_ADC_CAL` guard; targets
without it read through `analogReadMilliVolts()`, which applies the current
`esp_adc` calibration scheme (curve fitting where the chip supports it). This
matters rather than being cosmetic: at 12 dB attenuation the C6's ADC is
non-linear enough that the uncalibrated `raw × Vref / 4095` formula reads a
full battery well below its real voltage. Still worth sanity-checking the
reported voltage against a multimeter — `--set power.adc_multiplier_override`
trims it without a rebuild.

**Buzzer drive strength.** GPIO3 is a low-power pad with ~5 mA default drive,
which leaves the passive piezo quiet. The variant's `lateInitVariant()` raises
it to `GPIO_DRIVE_CAP_3` at boot.

**RTC drift.** The DS1307 has no temperature compensation and no digital
trimming — accuracy is whatever the external 32.768 kHz crystal gives, typically
around ±20 ppm at room temperature (roughly ±10 minutes/year) and worse across
temperature. Time is corrected whenever the node gets it from NTP or the mesh.

---

## 8. Updating to a new Meshtastic release

Because the port is a branch, updating is a rebase.

The tree may be a shallow clone (`--depth 1`), which has no history to rebase
against. Deepen it once:

```bash
git fetch upstream --unshallow
```

Then, for each new release:

```bash
git fetch upstream --tags
git checkout jppdevice
git rebase <new-tag>
git submodule update --init --depth 1
pio run -e jppdevice
```

Conflicts, if any, will land in the files the port patches — most likely
`RTC.cpp`, `ScanI2CTwoWire.cpp`, `InputBroker.cpp` and `Power.cpp`. The variant
directory is self-contained and should never conflict.

After rebasing, re-check the two upstream-shaped assumptions the port relies on,
since either could change:

- `MAX17048Sensor` still needing `MESHTASTIC_EXCLUDE_POWER_TELEMETRY` when
  environmental sensors are excluded
- `Power.cpp` still using the legacy `esp_adc_cal_*` API (if upstream migrates
  to `esp_adc/adc_cali.h`, drop the `HAS_ESP_ADC_CAL` commit entirely)

Check the [release notes](https://github.com/meshtastic/firmware/releases) for
breaking changes before rebasing across a minor version.
