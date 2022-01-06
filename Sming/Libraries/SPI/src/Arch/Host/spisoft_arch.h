/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * softspi_arch.h - Host
 *
 */

#pragma once

#define GP_IN(pin) digitalRead(pin)
#define GP_OUT(pin, val) digitalWrite(pin, (val)&1)

namespace spisoft
{
constexpr SpiPins defaultPins{
	.sck = 14,
	.miso = 12,
	.mosi = 13,
};

} // namespace spisoft
