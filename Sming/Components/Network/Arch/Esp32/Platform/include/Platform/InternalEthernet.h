/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * InternalEthernet.h
 *
 ****/

#pragma once

#include <Platform/Ethernet.h>

/**
 * @brief Ethernet provider using ESP32 internal MAC
 */
class InternalEthernet : public Ethernet
{
public:
	bool begin() override;
};

/** @} */
