/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * softspi_arch.h - Rp2040
 *
 */

#pragma once

#include <hardware/gpio.h>
#include <fast_io.h>

#define GP_IN(pin) GP_FAST_READ(pin, SIO_BASE + SIO_GPIO_IN_OFFSET)
#define GP_OUT(pin, val) GP_FAST_WRITE(pin, val, SIO_BASE + SIO_GPIO_OUT_CLR_OFFSET, SIO_BASE + SIO_GPIO_OUT_SET_OFFSET)

#define SPISOFT_ARCH_DELAY_FIXED 1

namespace spisoft
{
constexpr SpiPins defaultPins{
	.sck = PICO_DEFAULT_SPI_SCK_PIN,
	.miso = PICO_DEFAULT_SPI_RX_PIN,
	.mosi = PICO_DEFAULT_SPI_TX_PIN,
};

} // namespace spisoft
