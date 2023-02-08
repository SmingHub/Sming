/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * spi_config.h
 * 
 * Common definitions for SPI ethernet devices
 *
 ****/

#include <esp_eth.h>
#include <esp_event.h>
#include <driver/gpio.h>
#include "driver/spi_master.h"

#if defined(SOC_ESP32)
#define DEFAULT_HOST SPI2_HOST
#define DEFAULT_PIN_CS 22
#define DEFAULT_PIN_INT 4
#define DEFAULT_PIN_RESET 5
#elif defined(SOC_ESP32S2)
#define DEFAULT_HOST SPI2_HOST
#define DEFAULT_PIN_CS 34
#define DEFAULT_PIN_INT 19
#define DEFAULT_PIN_RESET 18
#elif defined(SOC_ESP32C3)
#define DEFAULT_HOST SPI2_HOST
#define DEFAULT_PIN_CS 10
#define DEFAULT_PIN_INT 19
#define DEFAULT_PIN_RESET 18
#elif defined(SOC_ESP32S3)
#define DEFAULT_HOST SPI2_HOST
#define DEFAULT_PIN_CS 10
#define DEFAULT_PIN_INT 19
#define DEFAULT_PIN_RESET 18
#elif defined(SOC_ESP32C2)
#define DEFAULT_HOST SPI2_HOST
#define DEFAULT_PIN_CS 10
#define DEFAULT_PIN_INT 19
#define DEFAULT_PIN_RESET 18
#endif

#define CHECK_RET(err)                                                                                                 \
	if(ESP_ERROR_CHECK_WITHOUT_ABORT(err) != ESP_OK) {                                                                 \
		return false;                                                                                                  \
	}
