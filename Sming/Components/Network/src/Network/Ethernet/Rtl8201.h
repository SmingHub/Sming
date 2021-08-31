/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Rtl8201.h
 *
 ****/

#pragma once

#include <Platform/Ethernet.h>

namespace Ethernet
{
/**
 * @brief RTL 8201 PHY interface
 */
class Rtl8201 : public PhyFactory
{
public:
	using PhyFactory::PhyFactory;

	PhyInstance* create() override;
	void destroy(PhyInstance* inst) override;
};

} // namespace Ethernet
