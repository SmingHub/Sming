/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * spi_arch.h - Host
 */

#pragma once

static constexpr uint8_t SOC_SPI_PERIPH_NUM{3};

/**
 * @brief Identifies bus selection
 */
enum class SpiBus {
	INVALID = 0,
	MIN = 1,
	SPI1 = 1,
	SPI2 = 2,
	SPI3 = 3,
	MAX = SOC_SPI_PERIPH_NUM,
	DEFAULT = SPI1,
};
