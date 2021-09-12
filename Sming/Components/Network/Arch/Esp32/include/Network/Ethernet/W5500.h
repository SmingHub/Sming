/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * W5500.h
 *
 ****/

#pragma once

#include <Platform/IdfService.h>

namespace Ethernet
{
class W5500PhyFactory : public Ethernet::PhyFactory
{
public:
	using PhyFactory::PhyFactory;

	PhyInstance* create(const PhyConfig& config) override;
	void destroy(PhyInstance* inst) override;
};

/**
 * @brief Ethernet provider using W5500 SPI.
 * 
 */
class W5500Service : public Ethernet::IdfService
{
public:
	struct Config {
		Ethernet::PhyConfig phy;
		uint32_t clockSpeed = 16 * 1000000;
		int8_t spiHost = -1;
		int8_t chipSelectPin = Ethernet::PIN_DEFAULT;
		int8_t interruptPin = Ethernet::PIN_DEFAULT;
	};

	W5500Service() : IdfService(w5500PhyFactory)
	{
	}

	bool begin(const Config& config);

private:
	static W5500PhyFactory w5500PhyFactory;
};

} // namespace Ethernet
