# J++Device — hardware specification

Verified against a reference unit. Where this disagrees with a datasheet,
the measured value here wins.

## MCU

| Property | Value |
|---|---|
| Chip | ESP32-C6 |
| CPU frequency | 160 MHz |
| Framework | ESP-IDF v5.1+ |
| FreeRTOS tick rate | 1000 Hz |
| Console | native USB-Serial-JTAG |

The board's single USB-C port has **no UART bridge chip**, so UART0 is not
reachable from a host — the console is the C6's built-in USB-Serial-JTAG
peripheral.

## Pin assignment

### I2C bus (`I2C_NUM_0`, 400 kHz)

| Signal | GPIO |
|---|---|
| SCL | 19 |
| SDA | 20 |

Devices on this bus:
- **SSD1306 OLED** — address `0x3C`
- **DS1307 RTC** — address `0x68`

### SPI bus (`SPI2_HOST`)

| Signal | GPIO |
|---|---|
| MISO | 18 |
| MOSI | 15 |
| CLK | 14 |
| CS — SX1276 LoRa (CS1) | 6 |
| CS — SD card (CS2) | 7 |

### SX1276 LoRa module

| Signal | GPIO | Notes |
|---|---|---|
| SPI CS | 6 | 8 MHz SPI |
| DIO0 | 4 | interrupt |
| RST | 5 | active-low reset, 10 ms low + 10 ms settle before first SPI access |

**Band: 868 MHz only.** The RF front-end's matching network is fixed; the
radio must not be configured for another band.

### SD card module

| Signal | GPIO | Notes |
|---|---|---|
| SPI CS | 7 | 20 MHz SPI |
| Mount point | `/sdcard` | FATFS, LFN enabled |

### Battery monitor

| Property | Value |
|---|---|
| ADC channel | `ADC_CHANNEL_1` (GPIO 1) |
| Divider | 2× 220 kΩ in series (Vbat → R1 → GPIO1 → R2 → GND) |
| Multiplier | ×2 (equal divider) |
| Full voltage | 4200 mV (100 %) |
| Empty voltage | 3000 mV (0 %) |

### Keypad (5-key resistor ladder)

| Property | Value |
|---|---|
| ADC channel | `ADC_CHANNEL_2` (GPIO 2) |
| ADC resolution | 12-bit (0–4095) |
| ADC attenuation | 12 dB |
| Pull-up | chip-internal ~45 kΩ, **must be enabled** |

Bands measured on a reference unit — see §4.6 of `HARDWARE_SUMMARY.md` in the
companion `jppdos` repository, which is the verified source for this board's
analog values.

The internal pull-up is the top of the resistor ladder, not merely an input
option: without it the pin floats and ghost-presses. It must be applied *after*
the ADC is configured, because ADC setup resets the pad's pull configuration.

| Name | Ladder R | ADC raw centre | Band (upper bound) |
|---|---|---|---|
| LEFT / BACK | ≈0 Ω (short) | ~4 | < 250 |
| UP | ≈5.7 kΩ | ~492 | 250 – 805 |
| DOWN | ≈15.5 kΩ | ~1120 | 806 – 1411 |
| RIGHT / NEXT | ≈29 kΩ | ~1710 | 1412 – 2093 |
| SELECT / CONFIRM | ≈59 kΩ | ~2476 | 2094 – 2909 |
| No key | ≈148 kΩ bleed | ~3344 | ≥ 2910 |

Note the ascending-ADC order is LEFT, UP, DOWN, RIGHT, SELECT — it does **not**
follow the board's KEY1–KEY5 numbering. Match on thresholds, never on an index.

Pull-up resistance varies ±30 % chip-to-chip, so the bounds above are midpoints
between adjacent bands and may need re-tuning per unit.

### Buzzer

| Property | Value |
|---|---|
| GPIO | 3 |
| Driver | LEDC PWM |
| LEDC timer | `LEDC_TIMER_0` |
| LEDC channel | `LEDC_CHANNEL_0` |
| Timer resolution | 10-bit (`LEDC_TIMER_10_BIT`) |
| 50 % duty | 512 |
| Drive capability | `GPIO_DRIVE_CAP_3`, **must be raised** |

GPIO 3 is a low-power (LP) pad whose default drive is ~5 mA, which leaves the
passive piezo quiet and short of full voltage swing. Raise it with
`gpio_set_drive_capability()`; the setting lives in IO_MUX and survives the
LEDC attach. The buzzer is passive, so it needs a PWM tone, not a DC level.