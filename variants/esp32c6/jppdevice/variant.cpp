#include "configuration.h"

#include <Arduino.h>
#include <driver/gpio.h>

/**
 * Board-specific init, run from setup() after the LoRa radio is up.
 *
 * GPIO3 is a low-power (LP) pad and comes up with weak default drive (~5 mA),
 * which leaves the passive piezo quiet and short of full voltage swing. Raise
 * it to full strength.
 *
 * Doing this before anything attaches LEDC to the pin is fine: drive strength
 * lives in an IO_MUX field that the LEDC driver never touches, so the setting
 * survives the attach that Arduino's tone() performs on the first beep.
 */
void lateInitVariant()
{
#ifdef PIN_BUZZER
    gpio_set_drive_capability((gpio_num_t)PIN_BUZZER, GPIO_DRIVE_CAP_3);
#endif
}
