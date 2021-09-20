/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DM9051.h
 *
 ****/

#pragma once

#include "SpiService.h"

namespace Ethernet
{
class DM9051PhyFactory : public Ethernet::PhyFactory
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
class DM9051Service : public SpiService
{
public:
	DM9051Service() : SpiService(dm9051PhyFactory)
	{
	}

	bool begin(const Config& config);

private:
	static DM9051PhyFactory dm9051PhyFactory;
};

} // namespace Ethernet
