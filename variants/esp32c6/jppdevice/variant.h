#pragma once

// ── Serial ───────────────────────────────────────────────────────────────────
// The C6 has no Serial2; match the other esp32c6 variants and use UART1.
#define SERIAL_PRINT_PORT 1

// ── I2C ──────────────────────────────────────────────────────────────────────
#define I2C_SDA 20
#define I2C_SCL 19

// ── SPI bus (shared by LoRa + SD) ───────────────────────────────────────────
#define LORA_SCK  14
#define LORA_MISO 18
#define LORA_MOSI 15
// setupSDCard() in FSCommon.cpp brings up the shared bus via these names
#define SPI_SCK   14
#define SPI_MISO  18
#define SPI_MOSI  15

// ── SX1276 LoRa (RF95-compatible) ───────────────────────────────────────────
#define USE_RF95
#define LORA_CS    6
#define LORA_DIO0  4
#define LORA_RESET 5
#define LORA_DIO1  RADIOLIB_NC
#define LORA_DIO2  RADIOLIB_NC
#define RF95_MAX_POWER 20

// ── SSD1306 OLED (I2C, 0x3C, 128×64) ───────────────────────────────────────
#define HAS_SCREEN       1
#define USE_SSD1306
#define SCREEN_WIDTH     128
#define SCREEN_HEIGHT    64
#define SCREEN_I2C_ADDR  0x3C

// ── DS1307 RTC (I2C, 0x68) — custom driver ─────────────────────────────────
#define DS1307_RTC 0x68

// ── SD card (SPI, CS=7, 20 MHz) ────────────────────────────────────────────
#define HAS_SDCARD
#define SDCARD_CS         7
#define SD_SPI_FREQUENCY  20000000U

// ── Battery ADC (GPIO1 = ADC1_CH1, 2× equal divider) ───────────────────────
#define BATTERY_PIN                    1
// Only consumed on targets with the legacy esp_adc_cal API (see power.h);
// the C6 uses the analogRead() fallback. NB: on C6 the soc header's
// ADC1_GPIO1_CHANNEL is a bare int, so name the enum directly.
#define ADC_CHANNEL                    ADC1_CHANNEL_1
#define ADC_MULTIPLIER                 2.0f
#define BATTERY_SENSE_RESOLUTION_BITS  12
#define BATTERY_SENSE_SAMPLES          30

// ── Buzzer (GPIO3, LEDC) ───────────────────────────────────────────────────
#define PIN_BUZZER 3

// ── 5-key ADC resistor-ladder keyboard (GPIO2 = ADC1_CH2) ──────────────────
#define HAS_JPPDEVICE_KEYBOARD
#define JPPDEVICE_KB_PIN 2
// Upper-bound thresholds for 12-bit ADC readings:
#define JPPDEVICE_KEY_UP_MAX      500   // KEY1: raw < 500
#define JPPDEVICE_KEY_DOWN_MAX   1300   // KEY2: 500 – 1299
#define JPPDEVICE_KEY_LEFT_MAX   2000   // KEY3: 1300 – 1999
#define JPPDEVICE_KEY_RIGHT_MAX  2800   // KEY4: 2000 – 2799
#define JPPDEVICE_KEY_SELECT_MAX 3800   // KEY5: 2800 – 3799
// raw >= 3800 → no key

// ── Feature exclusions ──────────────────────────────────────────────────────
#define MESHTASTIC_EXCLUDE_GPS                  1
#define MESHTASTIC_EXCLUDE_ENVIRONMENTAL_SENSOR 1
// No fuel-gauge IC (battery level comes from the ADC divider above). Also
// required: MAX17048Sensor derives from VoltageSensor, which is compiled out
// by MESHTASTIC_EXCLUDE_ENVIRONMENTAL_SENSOR.
#define MESHTASTIC_EXCLUDE_POWER_TELEMETRY      1
