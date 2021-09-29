/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http:// github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Digital.cpp
 *
 * Based on https:// github.com/earlephilhower/arduino-pico/blob/master/cores/rp2040/wiring_digital.cpp
 *
 ****/

#include <Digital.h>
#include <esp_systemapi.h>
#include <hardware/gpio.h>
#include <Platform/Timers.h>

namespace
{
constexpr uint32_t PIN_COUNT = NUM_BANK0_GPIOS;

#define CHECK_PIN(pin_number, ...)                                                                                     \
	if(pin_number >= PIN_COUNT) {                                                                                      \
		SYSTEM_ERROR("ERROR: Illegal pin in %s (%d)", __FUNCTION__, pin_number);                                       \
		return __VA_ARGS__;                                                                                            \
	}

} // namespace

void pinMode(uint16_t pin, uint8_t mode)
{
	CHECK_PIN(pin)

	switch(mode) {
	case INPUT:
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_IN);
		gpio_disable_pulls(pin);
		break;
	case INPUT_PULLUP:
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_IN);
		gpio_pull_up(pin);
		gpio_put(pin, 0);
		break;
	case INPUT_PULLDOWN:
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_IN);
		gpio_pull_down(pin);
		gpio_put(pin, 1);
		break;
	case OUTPUT:
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_OUT);
		gpio_disable_pulls(pin);
		break;
	default:
		SYSTEM_ERROR("ERROR: Illegal pinMode mode (%d)", mode);
		return;
	}
}

bool isInputPin(uint16_t pin)
{
	CHECK_PIN(pin, false)

	return !gpio_is_dir_out(pin);
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	CHECK_PIN(pin)

	if(gpio_is_pulled_down(pin)) {
		gpio_set_dir(pin, val != LOW);
	} else if(gpio_is_pulled_up(pin)) {
		gpio_set_dir(pin, val != HIGH);
	} else {
		gpio_put(pin, val != LOW);
	}
}

uint8_t digitalRead(uint16_t pin)
{
	CHECK_PIN(pin, LOW)
	return gpio_get(pin);
}

void pullup(uint16_t pin)
{
	CHECK_PIN(pin)
	gpio_pull_up(pin);
}

void noPullup(uint16_t pin)
{
	CHECK_PIN(pin)
	gpio_disable_pulls(pin);
}

unsigned long pulseIn(uint16_t pin, uint8_t state, unsigned long timeout)
{
	CHECK_PIN(pin, 0)

	OneShotFastUs timeout_timer;
	timeout_timer.reset(timeout);

	// Wait for deassert, if needed
	while(gpio_get(pin) != !state) {
		if(timeout_timer.expired()) {
			return 0;
		}
	}

	// Wait for assert
	while(gpio_get(pin) != !!state) {
		if(timeout_timer.expired()) {
			return 0;
		}
	}

	OneShotFastUs timer;

	// Wait for deassert
	while(gpio_get(pin) != !state) {
		if(timeout_timer.expired()) {
			return 0;
		}
	}

	return timer.elapsedTicks();
}
