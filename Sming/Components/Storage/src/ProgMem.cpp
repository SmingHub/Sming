/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ProgMem.cpp
 *
 ****/

#include "include/Storage/ProgMem.h"
#include <esp_spi_flash.h>

namespace Storage
{
ProgMem progMem;

bool ProgMem::read(uint32_t address, void* dst, size_t size)
{
	size_t readCount = flashmem_read(dst, address, size);
	return readCount == size;
}

Partition ProgMem::ProgMemPartitionTable::add(const String& name, const void* flashPtr, size_t size,
											  Partition::FullType type)
{
	auto addr = flashmem_get_address(flashPtr);
	if(addr == 0) {
		return Partition{};
	}

	return CustomPartitionTable::add(name, type, addr, size, Partition::Flag::readOnly);
}

} // namespace Storage
