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

void attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE type)
{
}

void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, GPIO_INT_TYPE type)
{
}

void attachInterruptHandler(uint8_t pin, GPIO_INT_TYPE type)
{
}

void detachInterrupt(uint8_t pin)
{
}

void interruptMode(uint8_t pin, GPIO_INT_TYPE type)
{
}
