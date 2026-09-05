#include "configuration.h" // must come first: it defines DS1307_RTC via variant.h

#ifdef DS1307_RTC

#include "DS1307.h"

bool DS1307::begin(TwoWire &wire)
{
    _wire = &wire;

    // Verify the DS1307 responds at 0x68
    _wire->beginTransmission(DS1307_ADDR);
    if (_wire->endTransmission() != 0)
        return false;

    // Read the seconds register to check/clear the Clock Halt bit
    _wire->beginTransmission(DS1307_ADDR);
    _wire->write(REG_SECONDS);
    _wire->endTransmission();
    _wire->requestFrom(DS1307_ADDR, (uint8_t)1);
    if (!_wire->available())
        return false;

    uint8_t seconds = _wire->read();
    if (seconds & CH_BIT) {
        // Oscillator is halted — clear CH bit to start the clock
        _wire->beginTransmission(DS1307_ADDR);
        _wire->write(REG_SECONDS);
        _wire->write(seconds & ~CH_BIT);
        _wire->endTransmission();
    }

    return true;
}

bool DS1307::getDateTime(struct tm *t)
{
    if (!_wire)
        return false;

    _wire->beginTransmission(DS1307_ADDR);
    _wire->write(REG_SECONDS);
    if (_wire->endTransmission() != 0)
        return false;

    _wire->requestFrom(DS1307_ADDR, REG_COUNT);
    if (_wire->available() < REG_COUNT)
        return false;

    uint8_t raw[REG_COUNT];
    for (int i = 0; i < REG_COUNT; i++)
        raw[i] = _wire->read();

    // Registers: 0=seconds, 1=minutes, 2=hours, 3=day-of-week, 4=date, 5=month, 6=year
    t->tm_sec  = bcd2dec(raw[0] & 0x7F); // mask out CH bit
    t->tm_min  = bcd2dec(raw[1] & 0x7F);
    t->tm_hour = bcd2dec(raw[2] & 0x3F); // 24-hour mode, bits 5:0
    t->tm_wday = (raw[3] & 0x07) - 1;    // DS1307: 1=Sun..7=Sat → tm: 0=Sun..6=Sat
    t->tm_mday = bcd2dec(raw[4] & 0x3F);
    t->tm_mon  = bcd2dec(raw[5] & 0x1F) - 1; // DS1307: 1-12 → tm: 0-11
    t->tm_year = bcd2dec(raw[6]) + 100;       // DS1307: 00-99 → tm: years since 1900 (2000-2099)
    t->tm_isdst = -1;

    return true;
}

bool DS1307::setDateTime(const struct tm *t)
{
    if (!_wire)
        return false;

    _wire->beginTransmission(DS1307_ADDR);
    _wire->write(REG_SECONDS);
    _wire->write(dec2bcd(t->tm_sec) & 0x7F);   // seconds, CH=0 (oscillator running)
    _wire->write(dec2bcd(t->tm_min));            // minutes
    _wire->write(dec2bcd(t->tm_hour));           // hours (24h mode)
    _wire->write(dec2bcd(t->tm_wday + 1));       // day of week: tm 0-6 → DS1307 1-7
    _wire->write(dec2bcd(t->tm_mday));           // date
    _wire->write(dec2bcd(t->tm_mon + 1));        // month: tm 0-11 → DS1307 1-12
    _wire->write(dec2bcd(t->tm_year - 100));     // year: tm years since 1900 → DS1307 00-99

    return _wire->endTransmission() == 0;
}

#endif // DS1307_RTC
