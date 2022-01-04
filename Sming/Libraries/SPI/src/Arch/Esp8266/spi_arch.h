/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * spi_arch.h - Esp8266
 *
 */

#pragma once

/**
 * @brief Identifies bus selection
 * @addtogroup hw_spi
 *
 * @todo Add support for overlapped I/O using SPI0 pins.
 * Devices can be represented as SPI2, SPI3 and SPI4 as there are three CS available.
 */
enum class SpiBus {
	INVALID = 0,
	MIN = 1,
	SPI1 = 1,
	MAX = 1,
	DEFAULT = SPI1,
};
