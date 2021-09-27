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

#include "SpiService.h"

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
class W5500Service : public SpiService
{
public:
	W5500Service() : SpiService(w5500PhyFactory)
	{
	}

	bool begin(const Config& config);

private:
	static W5500PhyFactory w5500PhyFactory;
};

} // namespace Ethernet
