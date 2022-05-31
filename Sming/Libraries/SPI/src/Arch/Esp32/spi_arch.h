/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * spi_arch.h - Esp32
 *
 * Based on Arduino-esp32 code
 *
 * 	https://github.com/espressif/arduino-esp32/blob/master/libraries/SPI/src/SPI.h
 * 	https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-spi.h
 */

#pragma once

#include <soc/soc_caps.h>

/**
 * @brief Identifies bus selection
 * @ingroup hw_spi
 */
enum class SpiBus {
	INVALID = 0,
	MIN = 1,
	SPI1 = 1,
	FSPI = 1, // Attached to the flash (can use the same data lines but different SS)
	SPI2 = 2,
	HSPI = 2, // Normally mapped to pins 12 - 15, but can be matrixed to any pins
#if SOC_SPI_PERIPH_NUM > 2
	SPI3 = 3,
	VSPI = 3, // Normally attached to pins 5, 18, 19 and 23, but can be matrixed to any pins
#endif
	MAX = SOC_SPI_PERIPH_NUM,
	DEFAULT = SPI2,
};
