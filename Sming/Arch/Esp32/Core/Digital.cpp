/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Digital.cpp
 *
 ****/

#include <Digital.h>
#include <esp_systemapi.h>
#define gpio_drive_cap_t uint32_t
#include <hal/gpio_ll.h>
#include <driver/rtc_io.h>
#include <hal/rtc_io_ll.h>

void pinMode(uint16_t pin, uint8_t mode)
{
	if(pin >= GPIO_PIN_COUNT) {
		return; // Bad pin
	}

	auto gpio = gpio_num_t(pin);
	auto rtc = rtc_io_number_get(gpio);

	if(mode == ANALOG) {
		if(rtc < 0) {
			return; // Not RTC pin
		}

		auto& desc = rtc_io_desc[rtc];
		if(READ_PERI_REG(desc.reg) & desc.mux) {
			return; // Already in ADC mode
		}

		rtcio_ll_function_select(rtc, RTCIO_FUNC_RTC);
		rtcio_ll_input_disable(rtc);
		rtcio_ll_output_disable(rtc);
		rtcio_ll_pullup_disable(rtc);
		rtcio_ll_pulldown_disable(rtc);

		return;
	}

	if(rtc >= 0) {
		rtcio_ll_function_select(rtc, RTCIO_FUNC_DIGITAL);
		rtcio_ll_pulldown_disable(rtc);
		if(mode == INPUT_PULLUP) {
			rtcio_ll_pullup_enable(rtc);
		} else {
			rtcio_ll_pullup_disable(rtc);
		}
	}

	gpio_set_level(gpio, 0);
	gpio_ll_input_enable(&GPIO, gpio);
	gpio_ll_pulldown_dis(&GPIO, gpio);

	if(mode == OUTPUT_OPEN_DRAIN) {
		gpio_ll_od_enable(&GPIO, gpio);
	} else {
		gpio_ll_od_disable(&GPIO, gpio);
	}

	if(mode == OUTPUT || mode == OUTPUT_OPEN_DRAIN) {
		gpio_ll_output_enable(&GPIO, gpio);
		gpio_matrix_out(gpio, SIG_GPIO_OUT_IDX, false, false);
	} else {
		gpio_ll_output_disable(&GPIO, gpio);
	}

	if(mode == INPUT_PULLUP) {
		gpio_ll_pullup_en(&GPIO, gpio);
	} else {
		gpio_ll_pullup_dis(&GPIO, gpio);
	}

	PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[pin], PIN_FUNC_GPIO);
}

//Detect if pin is input
bool isInputPin(uint16_t pin)
{
	gpio_num_t pinNumber = (gpio_num_t)(pin & 0x1F);

	if(GPIO_REG_READ(GPIO_ENABLE_REG) & BIT(pinNumber)) {
		//pin is output - read the GPIO_OUT_REG register
		return false;
	}

	return true;
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	gpio_ll_set_level(&GPIO, gpio_num_t(pin), val);
}

uint8_t IRAM_ATTR digitalRead(uint16_t pin)
{
	return gpio_ll_get_level(&GPIO, gpio_num_t(pin));
}

void pullup(uint16_t pin)
{
	gpio_ll_pullup_en(&GPIO, gpio_num_t(pin));
}

void noPullup(uint16_t pin)
{
	gpio_ll_pullup_dis(&GPIO, gpio_num_t(pin));
}

/* Measures the length (in microseconds) of a pulse on the pin; state is HIGH
 * or LOW, the type of pulse to measure.
 * Max timeout is 27 seconds at 160MHz clock and 54 seconds at 80MHz clock */
#define WAIT_FOR_PIN_STATE(state)                                                                                      \
	while(digitalRead(pin) != (state)) {                                                                               \
		if(esp_get_ccount() - start_cycle_count > timeout_cycles) {                                                    \
			return 0;                                                                                                  \
		}                                                                                                              \
	}

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
	const uint32_t max_timeout_us = clockCyclesToMicroseconds(UINT_MAX);
	if(timeout > max_timeout_us) {
		timeout = max_timeout_us;
	}
	const uint32_t timeout_cycles = microsecondsToClockCycles(timeout);
	const uint32_t start_cycle_count = esp_get_ccount();
	WAIT_FOR_PIN_STATE(!state);
	WAIT_FOR_PIN_STATE(state);
	const uint32_t pulse_start_cycle_count = esp_get_ccount();
	WAIT_FOR_PIN_STATE(!state);
	return clockCyclesToMicroseconds(esp_get_ccount() - pulse_start_cycle_count);
}
