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
#include <hardware/adc.h>
#include <bitset>

#define PIN_ADC0 26
#define PIN_ADC1 27
#define PIN_ADC2 28
#define PIN_ADC3 29
#define PIN_TEMP 30 // Not a GPIO
#define ADC_TEMP 4

namespace
{
constexpr uint32_t PIN_COUNT = NUM_BANK0_GPIOS;

std::bitset<NUM_ADC_CHANNELS> adcInitFlags;

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

	if(pin >= PIN_ADC0 && pin <= PIN_TEMP) {
		// Next call to `analogRead` needs to re-initialise
		uint8_t channel = pin - PIN_ADC0;
		adcInitFlags[channel] = 0;
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

uint16_t analogRead(uint16_t pin)
{
	if(pin < PIN_ADC0 || pin > PIN_TEMP) {
		// Not an analogue pin
		return 0;
	}

	uint8_t channel = pin - PIN_ADC0;
	if((adc_hw->cs & ADC_CS_EN_BITS) == 0) {
		adc_init();
	}
	if(!adcInitFlags[channel]) {
		if(channel == ADC_TEMP) {
			adc_set_temp_sensor_enabled(true);
		} else {
			adc_gpio_init(pin);
		}
		adcInitFlags[channel] = true;
	}

	adc_select_input(channel);
	return adc_read();
}
