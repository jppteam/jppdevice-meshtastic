#include "JppDeviceKeyboard.h"

#ifdef HAS_JPPDEVICE_KEYBOARD

#include <Arduino.h>

static const char inputSourceName[] = "JppDeviceKB";

// Map key index (0–4) to InputBroker event codes
static const input_broker_event kKeyMap[] = {
    INPUT_BROKER_UP,     // KEY1
    INPUT_BROKER_DOWN,   // KEY2
    INPUT_BROKER_LEFT,   // KEY3 (BACK)
    INPUT_BROKER_RIGHT,  // KEY4 (NEXT)
    INPUT_BROKER_SELECT, // KEY5 (CONFIRM)
};

JppDeviceKeyboard *jppDeviceKeyboard = nullptr;

JppDeviceKeyboard::JppDeviceKeyboard(const char *name) : concurrency::OSThread(name)
{
    inputBroker->registerSource(this);
}

/**
 * Read the resistor-ladder ADC and return the key index (0–4) or -1 for no key.
 */
int JppDeviceKeyboard::readKey()
{
    int raw = analogRead(JPPDEVICE_KB_PIN);

    if (raw < JPPDEVICE_KEY_UP_MAX)      return 0;
    if (raw < JPPDEVICE_KEY_DOWN_MAX)    return 1;
    if (raw < JPPDEVICE_KEY_LEFT_MAX)    return 2;
    if (raw < JPPDEVICE_KEY_RIGHT_MAX)   return 3;
    if (raw < JPPDEVICE_KEY_SELECT_MAX)  return 4;
    return -1; // no key pressed
}

void JppDeviceKeyboard::sendKey(input_broker_event key)
{
    InputEvent e = {};
    e.source     = inputSourceName;
    e.inputEvent = key;
    notifyObservers(&e);
}

/**
 * Poll every 50 ms. Fire on the second consecutive matching reading (debounce).
 */
int32_t JppDeviceKeyboard::runOnce()
{
    int key = readKey();

    if (key == lastKey) {
        // Fire exactly once on the second consecutive match
        if (key >= 0 && ++sameCount == 2) {
            sendKey(kKeyMap[key]);
        }
        // After firing, sameCount keeps incrementing but no further events
    } else {
        lastKey   = key;
        sameCount = 0;
    }

    return 50; // ms until next poll
}

#endif // HAS_JPPDEVICE_KEYBOARD
