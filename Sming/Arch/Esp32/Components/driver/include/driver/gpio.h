/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gpio.h
 *
 ****/

/** @file gpio.h */

#pragma once

#include <c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/gpio.h>

/**
 * @defgroup gpio_driver GPIO driver
 * @ingroup drivers
 * @{
 */

//GPIO FUNCTIONS
#define ESP32_LOW 0x0
#define ESP32_HIGH 0x1

#define ESP32_INPUT 0x01
#define ESP32_OUTPUT 0x02
#define ESP32_PULLUP 0x04
#define ESP32_INPUT_PULLUP 0x05
#define ESP32_PULLDOWN 0x08
#define ESP32_INPUT_PULLDOWN 0x09
#define ESP32_OPEN_DRAIN 0x10
#define ESP32_OUTPUT_OPEN_DRAIN 0x12

#define ESP32_SPECIAL 0xF0
#define ESP32_FUNCTION_1 0x00
#define ESP32_FUNCTION_2 0x20
#define ESP32_FUNCTION_3 0x40
#define ESP32_FUNCTION_4 0x60
#define ESP32_FUNCTION_5 0x80
#define ESP32_FUNCTION_6 0xA0
#define ESP32_ANALOG 0xC0

//Interrupt Modes
#define ESP32_DISABLED 0x00
#define ESP32_RISING 0x01
#define ESP32_FALLING 0x02
#define ESP32_CHANGE 0x03
#define ESP32_ONLOW 0x04
#define ESP32_ONHIGH 0x05
#define ESP32_ONLOW_WE 0x0C
#define ESP32_ONHIGH_WE 0x0D

typedef struct {
	uint8_t reg;  /*!< GPIO register offset from DR_REG_IO_MUX_BASE */
	int8_t rtc;   /*!< RTC GPIO number (-1 if not RTC GPIO pin) */
	int8_t adc;   /*!< ADC Channel number (-1 if not ADC pin) */
	int8_t touch; /*!< Touch Channel number (-1 if not Touch pin) */
} esp32_gpioMux_t;

extern const esp32_gpioMux_t esp32_gpioMux[40];
extern const int8_t esp32_adc2gpio[20];

#define digitalPinIsValid(pin) ((pin) < 40 && esp32_gpioMux[(pin)].reg)
#define digitalPinCanOutput(pin) ((pin) < 34 && esp32_gpioMux[(pin)].reg)
#define digitalPinToRtcPin(pin) (((pin) < 40) ? esp32_gpioMux[(pin)].rtc : -1)
#define digitalPinToAnalogChannel(pin) (((pin) < 40) ? esp32_gpioMux[(pin)].adc : -1)
#define digitalPinToTouchChannel(pin) (((pin) < 40) ? esp32_gpioMux[(pin)].touch : -1)
#define digitalPinToDacChannel(pin) (((pin) == 25) ? 0 : ((pin) == 26) ? 1 : -1)

/** @} */

#ifdef __cplusplus
}
#endif
