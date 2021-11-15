/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * pins_arduino.h
 *
 ****/

// File name selected for compatibility

#pragma once

#include "peripheral.h"

#define EXTERNAL_NUM_INTERRUPTS 16
#define NUM_DIGITAL_PINS 40
#define NUM_ANALOG_INPUTS 16

#define analogInputToDigitalPin(p) (((p) < 20) ? (esp32_adc2gpio[(p)]) : -1)
#define digitalPinToInterrupt(p) (((p) < 40) ? (p) : -1)
#define digitalPinHasPWM(p) (p < 34)

constexpr uint8_t A0{36};

#define GPIO_REG_TYPE uint32_t

// We use maximum compatibility to standard Arduino logic.

#define digitalPinToPort(pin) (0)
#define digitalPinToBitMask(pin) (1UL << (pin))
#define digitalPinToTimer(pin) (NOT_ON_TIMER)
#define portOutputRegister(port) ((volatile uint32_t*)&GPO)
#define portInputRegister(port) ((volatile uint32_t*)&GPI)
#define portModeRegister(port) ((volatile uint32_t*)&GPE)