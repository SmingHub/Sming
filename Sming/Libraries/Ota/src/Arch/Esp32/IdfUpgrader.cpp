/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * IdfUpgrader.cpp
 *
 ****/

#include "include/Ota/IdfUpgrader.h"

namespace Ota
{
bool IdfUpgrader::begin(Partition partition, size_t size)
{
	if(partition.size() < size) {
		return false; // the requested size is too big...
	}

	writtenSoFar = 0;
	maxSize = size ?: partition.size();

	esp_err_t result = esp_ota_begin(convertToIdfPartition(partition), maxSize, &handle);

	return result == ESP_OK;
}

size_t IdfUpgrader::write(const uint8_t* buffer, size_t size)
{
	if(writtenSoFar + size > maxSize) {
		// cannot write more bytes than allowed
		return 0;
	}

	esp_err_t result = esp_ota_write(handle, buffer, size);
	if(result != ESP_OK) {
		// write failed
		return 0;
	}

	writtenSoFar += size;

	return size;
}

} // namespace Ota
