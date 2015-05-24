/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_INTERRUPTS_H_
#define _SMING_CORE_INTERRUPTS_H_

#include "../Wiring/WiringFrameworkDependencies.h"
#include "../SmingCore/Delegate.h"

#define ESP_MAX_INTERRUPTS 16

typedef void (*InterruptCallback)(void);
extern InterruptCallback _gpioInterruptsList[ESP_MAX_INTERRUPTS];
extern bool _gpioInterruptsInitialied;

void attachInterrupt(uint8_t pin, InterruptCallback callback, uint8_t mode);
void attachInterrupt(uint8_t pin, Delegate<void()> delegateFunction, uint8_t mode);
void attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE mode); // ESP compatible version
void attachInterrupt(uint8_t pin, Delegate<void()> delegateFunction, GPIO_INT_TYPE mode); // ESP compatible version
void attachInterruptHandler(uint8_t pin, GPIO_INT_TYPE mode);
void detachInterrupt(uint8_t pin);
void interruptMode(uint8_t pin, uint8_t mode);
void interruptMode(uint8_t pin, GPIO_INT_TYPE type);
GPIO_INT_TYPE ConvertArduinoInterruptMode(uint8_t mode);

static void interruptHandler(uint32 intr_mask, void *arg);

// Disable interrupts
void noInterrupts();
// Enable interrupts
void interrupts();

#define digitalPinToInterrupt(pin)  ( (p) < ESP_MAX_INTERRUPTS ? (p) : -1 )

#define cli() noInterrupts()
#define sei() interrupts()

#endif /* _SMING_CORE_INTERRUPTS_H_ */
