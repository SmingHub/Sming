/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * IdfUpgrader.h
 *
 * This header includes all unified Over-The-Air functions.
 *
*/

#pragma once
#include <Ota/UpgraderBase.h>
#include <esp_ota_ops.h>

namespace Ota
{
/**
 * @brief ESP32 OTA Upgrader implementation
 */
class IdfUpgrader : public UpgraderBase
{
public:
	bool begin(Partition partition, size_t size = 0) override;
	size_t write(const uint8_t* buffer, size_t size) override;

	bool end() override
	{
		return esp_ota_end(handle) == ESP_OK;
	}

	bool abort() override
	{
		return true;
	}

	bool setBootPartition(Partition partition, bool save = true) override
	{
		if(!save) {
			return false;
		}

		return esp_ota_set_boot_partition(convertToIdfPartition(partition)) == ESP_OK;
	}

	Partition getBootPartition() override
	{
		return convertFromIdfPartition(esp_ota_get_boot_partition());
	}

	Partition getRunningPartition() override
	{
		return convertFromIdfPartition(esp_ota_get_running_partition());
	}

	Partition getNextBootPartition(Partition startFrom = {}) override
	{
		const esp_partition_t* idfFrom = startFrom ? convertToIdfPartition(startFrom) : nullptr;
		return convertFromIdfPartition(esp_ota_get_next_update_partition(idfFrom));
	}

	static const esp_partition_t* convertToIdfPartition(Partition partition)
	{
		return esp_partition_find_first(esp_partition_type_t(partition.type()),
										esp_partition_subtype_t(partition.subType()), partition.name().c_str());
	}

	static Partition convertFromIdfPartition(const esp_partition_t* partition)
	{
		return partition ? Storage::findPartition(String(partition->label)) : Partition{};
	}

private:
	size_t maxSize{0};
	size_t writtenSoFar{0};
	esp_ota_handle_t handle{};
};

} // namespace Ota
