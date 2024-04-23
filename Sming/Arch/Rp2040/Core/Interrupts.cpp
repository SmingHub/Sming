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

namespace
{
constexpr unsigned PIN_COUNT = NUM_BANK0_GPIOS;

struct Handler {
	enum class Type {
		none,
		interrupt,
		delegate,
	};
	union {
		InterruptCallback interrupt;
		InterruptDelegate* delegate;
	};
	Type type;

	void reset()
	{
		if(type == Type::delegate) {
			delete delegate;
		}
		type = Type::none;
	}

	void setInterrupt(InterruptCallback interrupt)
	{
		reset();
		this->interrupt = interrupt;
		type = Type::interrupt;
	}

	void setDelegate(InterruptDelegate delegate)
	{
		reset();
		this->delegate = new InterruptDelegate(std::move(delegate));
		type = Type::delegate;
	}
};

Handler handlers[PIN_COUNT]{};
bool interruptHandlerAttached;

#define CHECK_PIN(pin_number, ...)                                                                                     \
	if(pin_number >= PIN_COUNT) {                                                                                      \
		SYSTEM_ERROR("ERROR: Illegal pin in %s (%d)", __FUNCTION__, pin_number);                                       \
		return __VA_ARGS__;                                                                                            \
	}

void interruptDelegateCallback(uint32_t gpio)
{
	auto& handler = handlers[gpio];
	if(handler.type == Handler::Type::delegate) {
		(*handler.delegate)();
	}
}

void IRAM_ATTR interruptHandler(uint gpio, uint32_t events)
{
	auto& handler = handlers[gpio];
	if(handler.type == Handler::Type::interrupt) {
		handler.interrupt();
	} else if(handler.type == Handler::Type::delegate) {
		System.queueCallback(interruptDelegateCallback, gpio);
	}
}

} // namespace

void attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE type)
{
	CHECK_PIN(pin)

	auto& handler = handlers[pin];
	handler.setInterrupt(callback);
	attachInterruptHandler(pin, type);
}

void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, GPIO_INT_TYPE type)
{
	CHECK_PIN(pin)

	auto& handler = handlers[pin];
	handler.setDelegate(std::move(delegateFunction));
	attachInterruptHandler(pin, type);
}

void attachInterruptHandler(uint8_t pin, GPIO_INT_TYPE type)
{
	CHECK_PIN(pin)

	pinMode(pin, INPUT);

	if(interruptHandlerAttached) {
		gpio_set_irq_enabled(pin, type, true);
	} else {
		gpio_set_irq_enabled_with_callback(pin, type, true, interruptHandler);
		interruptHandlerAttached = true;
	}
}

void detachInterrupt(uint8_t pin)
{
	CHECK_PIN(pin)

	gpio_set_irq_enabled(pin, 0, false);
	handlers[pin].reset();
}

void interruptMode(uint8_t pin, GPIO_INT_TYPE type)
{
	CHECK_PIN(pin)

	pinMode(pin, INPUT);
	gpio_set_irq_enabled(pin, type, type != 0);
}
