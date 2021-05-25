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
bool IdfUpgrader::begin(Storage::Partition partition, size_t size)
{
	if(partition.size() < size) {
		return false; // the requested size is too big...
	}

	writtenSoFar = 0;
	maxSize = (size ? size : partition.size());

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

bool IdfUpgrader::setBootPartition(Storage::Partition partition)
{
	return esp_ota_set_boot_partition(convertToIdfPartition(partition)) == ESP_OK;
}

Storage::Partition IdfUpgrader::getBootPartition(void)
{
	return convertFromIdfPartition(esp_ota_get_boot_partition());
}

Storage::Partition IdfUpgrader::getRunningPartition(void)
{
	return convertFromIdfPartition(esp_ota_get_running_partition());
}

Storage::Partition IdfUpgrader::getNextBootPartition(Storage::Partition* startFrom)
{
	const esp_partition_t* idfFrom = nullptr;
	if(startFrom != nullptr) {
		idfFrom =convertToIdfPartition(*startFrom);
	}
	return convertFromIdfPartition(esp_ota_get_next_update_partition(idfFrom));
}

const esp_partition_t* IdfUpgrader::convertToIdfPartition(Storage::Partition partition)
{
	return esp_partition_find_first(ESP_PARTITION_TYPE_APP,
									partition.subType() == uint8_t(Storage::Partition::SubType::App::ota0)
										? ESP_PARTITION_SUBTYPE_APP_OTA_0
										: ESP_PARTITION_SUBTYPE_APP_OTA_1,
									partition.name().c_str());
}

Storage::Partition IdfUpgrader::convertFromIdfPartition(const esp_partition_t* partition)
{
	String label;
	if(partition != nullptr) {
		label = partition->label;
	}

	return Storage::findPartition(label);
}

} // namespace Ota
