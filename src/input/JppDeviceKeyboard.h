#pragma once

#include "configuration.h"

#ifdef HAS_JPPDEVICE_KEYBOARD

#include "InputBroker.h"
#include "concurrency/OSThread.h"

/**
 * ADC resistor-ladder 5-key keyboard driver for J++Device.
 *
 * Polls ADC1 channel 2 (GPIO2) every 50 ms. Two consecutive identical
 * readings are required before dispatching an event (simple debounce).
 *
 * Modeled after ExpressLRSFiveWay — extends Observable<const InputEvent *>
 * and registers itself with InputBroker on construction.
 */
class JppDeviceKeyboard : public Observable<const InputEvent *>, public concurrency::OSThread
{
  public:
    explicit JppDeviceKeyboard(const char *name);

  protected:
    int32_t runOnce() override;

  private:
    bool padReady = false;
    int lastKey = -1;
    int sameCount = 0;

    void setupPad();
    int readKey();
    void sendKey(input_broker_event key);
};

extern JppDeviceKeyboard *jppDeviceKeyboard;

#endif // HAS_JPPDEVICE_KEYBOARD
