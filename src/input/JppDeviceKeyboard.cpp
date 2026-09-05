#include "JppDeviceKeyboard.h"

#ifdef HAS_JPPDEVICE_KEYBOARD

#include <Arduino.h>
#include <driver/gpio.h>

static const char inputSourceName[] = "JppDeviceKB";

// Resistor-ladder bands, ordered by ascending ADC reading. The keys are not
// numbered on the board in this order — LEFT is the near-zero-ohm short at the
// bottom of the ladder — so always match on the thresholds, never on a key
// index.
struct JppKeyBand {
    int maxRaw;
    input_broker_event event;
};

static const JppKeyBand kBands[] = {
    {JPPDEVICE_KEY_LEFT_MAX, INPUT_BROKER_LEFT},   {JPPDEVICE_KEY_UP_MAX, INPUT_BROKER_UP},
    {JPPDEVICE_KEY_DOWN_MAX, INPUT_BROKER_DOWN},   {JPPDEVICE_KEY_RIGHT_MAX, INPUT_BROKER_RIGHT},
    {JPPDEVICE_KEY_SELECT_MAX, INPUT_BROKER_SELECT},
};
static const int kBandCount = sizeof(kBands) / sizeof(kBands[0]);

JppDeviceKeyboard *jppDeviceKeyboard = nullptr;

JppDeviceKeyboard::JppDeviceKeyboard(const char *name) : concurrency::OSThread(name)
{
    inputBroker->registerSource(this);
}

/**
 * Bring up the ADC pad.
 *
 * The chip's internal pull-up is the top of the resistor ladder, so it has to
 * be applied *after* the ADC has claimed the pin: configuring the ADC resets
 * the pad's pull configuration, and without the pull-up the pin floats and
 * ghost-presses.
 */
void JppDeviceKeyboard::setupPad()
{
    analogSetPinAttenuation(JPPDEVICE_KB_PIN, ADC_11db); // 12 dB, so the full ladder fits
    (void)analogRead(JPPDEVICE_KB_PIN);                  // force ADC init on this pin
    gpio_set_pull_mode((gpio_num_t)JPPDEVICE_KB_PIN, GPIO_PULLUP_ONLY);
    padReady = true;
}

/**
 * Read the ladder and return the index of the band the reading falls into, or
 * -1 when no key is down.
 */
int JppDeviceKeyboard::readKey()
{
    int raw = analogRead(JPPDEVICE_KB_PIN);

    for (int i = 0; i < kBandCount; i++) {
        if (raw < kBands[i].maxRaw)
            return i;
    }
    return -1; // above the top band: ladder bleed, nothing pressed
}

void JppDeviceKeyboard::sendKey(input_broker_event key)
{
    InputEvent e = {};
    e.source = inputSourceName;
    e.inputEvent = key;
    notifyObservers(&e);
}

/**
 * Poll every 50 ms. Fire on the second consecutive matching reading (debounce).
 */
int32_t JppDeviceKeyboard::runOnce()
{
    if (!padReady)
        setupPad();

    int key = readKey();

    if (key == lastKey) {
        // Fire exactly once on the second consecutive match
        if (key >= 0 && ++sameCount == 2) {
            sendKey(kBands[key].event);
        }
        // After firing, sameCount keeps incrementing but no further events
    } else {
        lastKey = key;
        sameCount = 0;
    }

    return 50; // ms until next poll
}

#endif // HAS_JPPDEVICE_KEYBOARD
