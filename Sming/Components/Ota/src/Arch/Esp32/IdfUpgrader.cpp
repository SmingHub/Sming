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

using namespace Storage;

namespace Ota
{
bool IdfUpgrader::begin(Partition partition, size_t size)
{
	if(partition.size() < size) {
		return false; // the requested size is too big...
	}

	writtenSoFar = 0;
	maxSize = size ?: partition.size();

	esp_err_t result = esp_ota_begin(convertToIdfPartition(partition), size ? size : partition.size(), &handle);

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

bool IdfUpgrader::end()
{
	return esp_ota_end(handle) == ESP_OK;
}

bool IdfUpgrader::abort()
{
	return true;
}

bool IdfUpgrader::setBootPartition(Partition partition)
{
	return esp_ota_set_boot_partition(convertToIdfPartition(partition)) == ESP_OK;
}

Partition IdfUpgrader::getBootPartition(void)
{
	return convertFromIdfPartition(esp_ota_get_boot_partition());
}

Partition IdfUpgrader::getRunningPartition(void)
{
	return convertFromIdfPartition(esp_ota_get_running_partition());
}

Partition IdfUpgrader::getNextBootPartition(Partition startFrom)
{
	const esp_partition_t* idfFrom = startFrom ? convertToIdfPartition(startFrom) : nullptr;
	return convertFromIdfPartition(esp_ota_get_next_update_partition(idfFrom));
}

const esp_partition_t* IdfUpgrader::convertToIdfPartition(Partition partition)
{
	return esp_partition_find_first(esp_partition_type_t(partition.type()),
									esp_partition_subtype_t(partition.subType()), partition.name().c_str());
}

Partition IdfUpgrader::convertFromIdfPartition(const esp_partition_t* partition)
{
	return partition ? findPartition(String(partition->label)) : Partition{};
}

} // namespace Ota
