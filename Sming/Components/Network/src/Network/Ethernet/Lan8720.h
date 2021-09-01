/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Lan8720.h
 *
 ****/

#pragma once

#include <Platform/Ethernet.h>

namespace Ethernet
{
/**
 * @brief LAN 8720 PHY interface
 */
class Lan8720 : public PhyFactory
{
public:
	using PhyFactory::PhyFactory;

	PhyInstance* create(const PhyConfig& config) override;
	void destroy(PhyInstance* inst) override;
};

} // namespace Ethernet
