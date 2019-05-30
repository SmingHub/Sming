/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * pins_arduino.h
 *
 ****/

#pragma once

const uint16_t A0 = 9999;

//#define NOT_A_PIN 0
//#define NOT_A_PORT 0
//#define NOT_ON_TIMER 0
//
//#define PA 1
//#define PB 2
//#define PC 3

typedef uint32_t GPIO_REG_TYPE;

// We use maximum compatibility to standard Arduino logic.

//#define digitalPinToPort(pin) (0)
//#define digitalPinToBitMask(pin) (1UL << (pin))
//#define digitalPinToTimer(pin) (NOT_ON_TIMER)
//#define portOutputRegister(port) ((volatile uint32_t*)&GPO)
//#define portInputRegister(port) ((volatile uint32_t*)&GPI)
//#define portModeRegister(port) ((volatile uint32_t*)&GPE)
