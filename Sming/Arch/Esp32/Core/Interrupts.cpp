/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Interrupts.cpp
 *
 ****/

#include <esp_systemapi.h>
#include <Interrupts.h>
#include <Digital.h>
#include <Platform/System.h>
#include <BitManipulations.h>
#include <esp_intr_alloc.h>
#include <driver/gpio.h>

constexpr unsigned MAX_INTERRUPTS = 40;

static intr_handle_t interruptHandle = nullptr;

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

static inline void IRAM_ATTR interruptProcessor(uint32_t status, const uint32_t gpio_num_start)
{
	while(status) {
		int nbit = __builtin_ffs(status) - 1;
		status &= ~(1 << nbit);
		int pin = gpio_num_start + nbit;
		if(gpioInterruptsList[pin]) {
			gpioInterruptsList[pin]();
		} else if(delegateFunctionList[pin]) {
			System.queueCallback(interruptDelegateCallback, pin);
		}
	}
}

/** @brief  Interrupt handler
 *  @param  arg pointer to array of arguments
 */
static void IRAM_ATTR interruptHandler(void* arg)
{
	if(!interruptHandlerAttached) {
		return;
	}

	int cpuId = esp_intr_get_cpu(interruptHandle);

	//read status to get interrupt status for GPIO0-31
	const uint32_t statusLow = (cpuId == 0) ? GPIO.pcpu_int : GPIO.acpu_int;
	if(statusLow) {
		interruptProcessor(statusLow, 0);
		GPIO.status_w1tc = statusLow;
	}

	//read status1 to get interrupt status for GPIO32-39
	const uint32_t statusHigh = (cpuId == 0) ? GPIO.pcpu_int1.intr : GPIO.acpu_int1.intr;
	if(statusHigh) {
		interruptProcessor(statusHigh, 32);
		GPIO.status1_w1tc.intr_st = statusHigh;
	}
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
	if(!interruptHandlerAttached) {
		gpio_isr_register(interruptHandler, NULL, (int)ESP_INTR_FLAG_IRAM, &interruptHandle);
		interruptHandlerAttached = true;
	}

	gpio_intr_disable((gpio_num_t)pin);

	pinMode(pin, INPUT);

	gpio_set_intr_type((gpio_num_t)pin, (gpio_int_type_t)type);
	gpio_intr_enable((gpio_num_t)pin);
}

void detachInterrupt(uint8_t pin)
{
	gpioInterruptsList[pin] = nullptr;
	delegateFunctionList[pin] = nullptr;
	attachInterruptHandler(pin, GPIO_PIN_INTR_DISABLE);
}

// @TODO: remove the function below...
void interruptMode(uint8_t pin, GPIO_INT_TYPE type)
{
	gpio_intr_disable((gpio_num_t)pin);

	pinMode(pin, INPUT);

	gpio_set_intr_type((gpio_num_t)pin, (gpio_int_type_t)type);
	gpio_intr_enable((gpio_num_t)pin);
}
