/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Interrupts.cpp
 *
 ****/

#include "Interrupts.h"
#include "Digital.h"
#include "WiringFrameworkIncludes.h"
#include "Platform/System.h"

static InterruptCallback gpioInterruptsList[16] = {0};
static InterruptDelegate delegateFunctionList[16];
static bool gpioInterruptsInitialied = false;

/** @brief  Interrupt handler
 *  @param  intr_mask Interrupt mask
 *  @param  arg pointer to array of arguments
 */
static void IRAM_ATTR interruptHandler(uint32 intr_mask, void* arg)
{
	bool processed;
	uint32 gpioStatus;

	do {
		gpioStatus = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
		processed = false;
		for(uint8 i = 0; i < ESP_MAX_INTERRUPTS; i++) {
			if(!bitRead(gpioStatus, i)) {
				continue;
			}

			// clear interrupt status
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpioStatus & _BV(i));

			if(gpioInterruptsList[i]) {
				gpioInterruptsList[i]();
			} else if(delegateFunctionList[i]) {
				System.queueCallback(
					[](uint32_t interruptNumber) {
						auto& delegate = delegateFunctionList[interruptNumber];
						if(delegate)
							delegate();
					},
					i);
			}

			processed = true;
		}
	} while(processed);
}

void attachInterrupt(uint8_t pin, InterruptCallback callback, uint8_t mode)
{
	GPIO_INT_TYPE type = ConvertArduinoInterruptMode(mode);
	attachInterrupt(pin, callback, type);
}

void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, uint8_t mode)
{
	GPIO_INT_TYPE type = ConvertArduinoInterruptMode(mode);
	attachInterrupt(pin, delegateFunction, type);
}

void attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE mode)
{
	if(pin >= 16)
		return; // WTF o_O
	gpioInterruptsList[pin] = callback;
	delegateFunctionList[pin] = nullptr;
	attachInterruptHandler(pin, mode);
}

void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, GPIO_INT_TYPE mode)
{
	if(pin >= 16)
		return; // WTF o_O
	gpioInterruptsList[pin] = nullptr;
	delegateFunctionList[pin] = delegateFunction;
	attachInterruptHandler(pin, mode);
}

void attachInterruptHandler(uint8_t pin, GPIO_INT_TYPE mode)
{
	ETS_GPIO_INTR_DISABLE();

	if(!gpioInterruptsInitialied) {
		ETS_GPIO_INTR_ATTACH((ets_isr_t)interruptHandler, nullptr); // Register interrupt handler
		gpioInterruptsInitialied = true;
	}

	pinMode(pin, INPUT);

	gpio_pin_intr_state_set(GPIO_ID_PIN(pin), mode); // Enable GPIO pin interrupt

	ETS_GPIO_INTR_ENABLE();
}

void detachInterrupt(uint8_t pin)
{
	gpioInterruptsList[pin] = nullptr;
	delegateFunctionList[pin] = nullptr;
	attachInterruptHandler(pin, GPIO_PIN_INTR_DISABLE);
}

void interruptMode(uint8_t pin, uint8_t mode)
{
	GPIO_INT_TYPE type = ConvertArduinoInterruptMode(mode);
	interruptMode(pin, type);
}

void interruptMode(uint8_t pin, GPIO_INT_TYPE type)
{
	ETS_GPIO_INTR_DISABLE();

	pinMode(pin, INPUT);

	gpio_pin_intr_state_set(GPIO_ID_PIN(pin), type);

	ETS_GPIO_INTR_ENABLE();
}

GPIO_INT_TYPE ConvertArduinoInterruptMode(uint8_t mode)
{
	switch(mode) {
	case LOW: // to trigger the interrupt whenever the pin is low,
		return GPIO_PIN_INTR_LOLEVEL;
	case CHANGE:				 // to trigger the interrupt whenever the pin changes value
		return (GPIO_INT_TYPE)3; // GPIO_PIN_INTR_ANYEDGE
	case RISING:				 // to trigger when the pin goes from low to high,
		return GPIO_PIN_INTR_POSEDGE;
	case FALLING: // for when the pin goes from high to low.
		return GPIO_PIN_INTR_NEGEDGE;
	case HIGH: // to trigger the interrupt whenever the pin is high.
		return GPIO_PIN_INTR_HILEVEL;
	default:
		return GPIO_PIN_INTR_DISABLE;
	}
}
