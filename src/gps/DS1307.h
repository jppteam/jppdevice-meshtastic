#pragma once

#include "configuration.h"

#ifdef DS1307_RTC

#include <Wire.h>
#include <time.h>

/**
 * Lightweight DS1307 RTC driver for Meshtastic.
 *
 * Communicates over I2C at address 0x68.
 * Reads/writes BCD-encoded time from registers 0x00–0x06.
 */
class DS1307
{
  public:
    /**
     * Initialize the DS1307 on the given I2C bus.
     * Verifies the chip responds at 0x68 and clears the Clock Halt (CH) bit
     * if set, so the oscillator is running.
     *
     * @return true if the chip responded and is ready.
     */
    bool begin(TwoWire &wire);

    /**
     * Read the current date/time from the DS1307 into a struct tm.
     * Fields populated: tm_sec, tm_min, tm_hour, tm_mday, tm_mon (0-based),
     * tm_year (years since 1900), tm_wday (0=Sunday).
     *
     * @return true if the read succeeded.
     */
    bool getDateTime(struct tm *t);

    /**
     * Write a date/time to the DS1307.
     *
     * @return true if the write succeeded.
     */
    bool setDateTime(const struct tm *t);

  private:
    static constexpr uint8_t DS1307_ADDR = 0x68;
    static constexpr uint8_t REG_SECONDS = 0x00;
    static constexpr uint8_t REG_COUNT   = 7;  // seconds..year
    static constexpr uint8_t CH_BIT      = 0x80; // Clock Halt bit in seconds register

    TwoWire *_wire = nullptr;

    static uint8_t bcd2dec(uint8_t bcd) { return (bcd >> 4) * 10 + (bcd & 0x0F); }
    static uint8_t dec2bcd(uint8_t dec) { return ((dec / 10) << 4) | (dec % 10); }
};

#endif // DS1307_RTC
