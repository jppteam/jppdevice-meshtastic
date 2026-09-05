#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

// Default serial
static const uint8_t TX = 16;
static const uint8_t RX = 17;

// I2C — SSD1306 OLED + DS1307 RTC
static const uint8_t SDA = 20;
static const uint8_t SCL = 19;

// Default SPI — mapped to LoRa (SX1276)
static const uint8_t SS   = 6;
static const uint8_t MOSI = 15;
static const uint8_t MISO = 18;
static const uint8_t SCK  = 14;

#endif /* Pins_Arduino_h */
