/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

// File name selected for compatibility

#ifndef WIRING_PINS_ARDUINO_H_
#define WIRING_PINS_ARDUINO_H_

#define NOT_A_PIN 0
#define NOT_A_PORT 0
#define NOT_ON_TIMER 0

#define PA 1
#define PB 2
#define PC 3

#define GPIO_REG_TYPE uint8_t

// We use maximum compatibility to standard Arduino logic.

// Conversion disabled for now
#define digitalPinToTimer(P) ( NOT_ON_TIMER )

#define digitalPinToPort(P) ( P < 0 ? NOT_A_PIN : ( (int)P < 8 ? PA : ( (int)P < 16 ? PB : ( (int)P == 16 ? PC : NOT_A_PIN ) ) ) )
#define digitalPinToBitMask(P) ( (int)P < 8 ? _BV((int)P) : ( P < 16 ? _BV( (int)P-8 ) : 1) )

#define STD_GPIO_OUT (PERIPHS_GPIO_BASEADDR + GPIO_OUT_ADDRESS)
#define STD_GPIO_IN (PERIPHS_GPIO_BASEADDR + GPIO_IN_ADDRESS)
#define STD_GPIO_ENABLE (PERIPHS_GPIO_BASEADDR + GPIO_ENABLE_ADDRESS)

#define portOutputRegister(P) ( ((volatile uint8_t*)(P != PC ? STD_GPIO_OUT : RTC_GPIO_OUT)) + ( ( ((int)P) == PB ) ? 1 : 0) )
#define portInputRegister(P)  ( ((volatile uint8_t*)(P != PC ? STD_GPIO_IN : RTC_GPIO_IN_DATA)) + ( ( ((int)P) == PB ) ? 1 : 0) )
#define portModeRegister(P)	  ( ((volatile uint8_t*)(P != PC ? STD_GPIO_ENABLE : RTC_GPIO_ENABLE)) + ( ( ((int)P) == PB ) ? 1 : 0) ) // Stored bits: 0=In, 1=Out


#endif /* WIRING_PINS_ARDUINO_H_ */
