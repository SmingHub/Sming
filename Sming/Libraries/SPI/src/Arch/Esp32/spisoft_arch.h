/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * softspi_arch.h - Esp32
 *
 */

#pragma once

#include <hal/gpio_ll.h>
#include <soc/spi_pins.h>
#include <fast_io.h>

#define GP_IN(pin) GP_FAST_READ(pin, GPIO_IN_REG)
#define GP_OUT(pin, val) GP_FAST_WRITE(pin, val, GPIO_OUT_W1TC_REG, GPIO_OUT_W1TS_REG)

namespace spisoft
{
constexpr SpiPins defaultPins{
	.sck = SPI2_IOMUX_PIN_NUM_CLK,
	.miso = SPI2_IOMUX_PIN_NUM_MISO,
	.mosi = SPI2_IOMUX_PIN_NUM_MOSI,
};

}
