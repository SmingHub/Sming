/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SpiService.h
 *
 ****/

#pragma once

#include <Platform/IdfService.h>

namespace Ethernet
{
/**
 * @brief SPI ethernet provider
 */
class SpiService : public IdfService
{
public:
	struct Config {
		Ethernet::PhyConfig phy;
		uint32_t clockSpeed = 16 * 1000000;
		int8_t spiHost = -1;
		int8_t chipSelectPin = Ethernet::PIN_DEFAULT;
		int8_t interruptPin = Ethernet::PIN_DEFAULT;
	};

	using IdfService::IdfService;
};

} // namespace Ethernet
