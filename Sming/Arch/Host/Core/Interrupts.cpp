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

void attachInterrupt(uint8_t pin, InterruptCallback callback, uint8_t mode)
{
}

void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, uint8_t mode)
{
}

void attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE mode)
{
}

void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, GPIO_INT_TYPE mode)
{
}

void attachInterruptHandler(uint8_t pin, GPIO_INT_TYPE mode)
{
}

void detachInterrupt(uint8_t pin)
{
}

void interruptMode(uint8_t pin, uint8_t mode)
{
}

void interruptMode(uint8_t pin, GPIO_INT_TYPE type)
{
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
