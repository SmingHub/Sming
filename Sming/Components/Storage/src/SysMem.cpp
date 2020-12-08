/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SysMem.cpp
 *
 ****/

#include "include/Storage/SysMem.h"
#include <esp_spi_flash.h>

namespace Storage
{
SysMem sysMem;

Partition SysMem::createPartition(const String& name, const FSTR::ObjectBase& fstr, Partition::Type type,
								  uint8_t subtype)
{
	return createPartition(name, type, subtype, reinterpret_cast<uint32_t>(fstr.data()), fstr.size(),
						   Partition::Flag::readOnly);
}

} // namespace Storage
