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
class IdfUpgrader : public UpgraderBase
{
public:
	/**
	 * @brief Prepare the partition for
	 */
	bool begin(Storage::Partition partition, size_t size = 0) override;
	size_t write(const uint8_t* buffer, size_t size) override;
	bool end() override;
	bool abort() override;

	bool setBootPartition(Storage::Partition partition) override;
	Storage::Partition getBootPartition(void) override;
	Storage::Partition getRunningPartition(void) override;
	Storage::Partition getNextBootPartition(Storage::Partition* startFrom = nullptr) override;

	static const esp_partition_t* convertToIdfPartition(Storage::Partition partition);
	static Storage::Partition convertFromIdfPartition(const esp_partition_t* partition);

private:
	size_t maxSize{0};
	size_t writtenSoFar{0};
	esp_ota_handle_t handle{};
};

} // namespace Ota
