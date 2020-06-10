/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Interrupts.cpp
 *
 ****/

#include <Interrupts.h>
#include <Digital.h>
#include <Platform/System.h>
#include <BitManipulations.h>

constexpr unsigned MAX_INTERRUPTS = 16;

static InterruptCallback gpioInterruptsList[MAX_INTERRUPTS] = {0};
static InterruptDelegate delegateFunctionList[MAX_INTERRUPTS];
static bool interruptHandlerAttached = false;

static void interruptDelegateCallback(uint32_t interruptNumber)
{
	auto& delegate = delegateFunctionList[interruptNumber];
	if(delegate) {
		delegate();
	}
}

/** @brief  Interrupt handler
 *  @param  intr_mask Interrupt mask
 *  @param  arg pointer to array of arguments
 */
static void IRAM_ATTR interruptHandler(uint32 intr_mask, void* arg)
{
	bool processed;

	do {
		uint32 gpioStatus = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
		processed = false;
		for(uint8 i = 0; i < MAX_INTERRUPTS; i++) {
			if(!bitRead(gpioStatus, i)) {
				continue;
			}

			// clear interrupt status
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, _BV(i));

			if(gpioInterruptsList[i]) {
				gpioInterruptsList[i]();
			} else if(delegateFunctionList[i]) {
				System.queueCallback(interruptDelegateCallback, i);
			}

			processed = true;
		}
	} while(processed);
}

void attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE type)
{
	if(pin >= MAX_INTERRUPTS) {
		return; // WTF o_O
	}
	gpioInterruptsList[pin] = callback;
	delegateFunctionList[pin] = nullptr;
	attachInterruptHandler(pin, type);
}

void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, GPIO_INT_TYPE type)
{
	if(pin >= MAX_INTERRUPTS) {
		return; // WTF o_O
	}
	gpioInterruptsList[pin] = nullptr;
	delegateFunctionList[pin] = delegateFunction;
	attachInterruptHandler(pin, type);
}

void attachInterruptHandler(uint8_t pin, GPIO_INT_TYPE type)
{
	ETS_GPIO_INTR_DISABLE();

	if(!interruptHandlerAttached) {
		ETS_GPIO_INTR_ATTACH((ets_isr_t)interruptHandler, nullptr); // Register interrupt handler
		interruptHandlerAttached = true;
	}

	pinMode(pin, INPUT);

	gpio_pin_intr_state_set(GPIO_ID_PIN(pin), type); // Enable GPIO pin interrupt

	ETS_GPIO_INTR_ENABLE();
}

void detachInterrupt(uint8_t pin)
{
	gpioInterruptsList[pin] = nullptr;
	delegateFunctionList[pin] = nullptr;
	attachInterruptHandler(pin, GPIO_PIN_INTR_DISABLE);
}

void interruptMode(uint8_t pin, GPIO_INT_TYPE type)
{
	ETS_GPIO_INTR_DISABLE();

	pinMode(pin, INPUT);

	gpio_pin_intr_state_set(GPIO_ID_PIN(pin), type);

	ETS_GPIO_INTR_ENABLE();
}
