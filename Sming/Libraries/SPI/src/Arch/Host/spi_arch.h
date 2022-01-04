/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * spi_arch.h - Host
 */

#pragma once

/**
 * @brief Identifies bus selection
 * @addtogroup hw_spi
 */
enum class SpiBus {
	INVALID = 0,
	MIN = 1,
	SPI1 = 1,
	MAX = 1,
	DEFAULT = SPI1,
};
