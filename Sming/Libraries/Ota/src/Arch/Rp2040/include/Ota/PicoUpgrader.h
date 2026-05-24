/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Ota.h
 *
 * This header includes all unified Over-The-Air functions.
 *
*/

#pragma once
#include <Ota/UpgraderBase.h>
#include <Storage/PartitionStream.h>

namespace Ota
{
/**
 * @brief ESP8266 rBoot OTA Upgrader implementation
 */
class PicoUpgrader : public UpgraderBase
{
public:
	/**
	 * @brief Prepare the partition for
	 */
	bool begin(Partition partition, size_t size = 0) override;
	size_t write(const uint8_t* buffer, size_t size) override;
	bool end() override;
	bool setBootPartition(Partition partition, bool save = true) override;
	Partition getBootPartition() override;
	Partition getRunningPartition() override;
	Partition getNextBootPartition(Partition startFrom = {}) override;

private:
	std::unique_ptr<Storage::PartitionStream> stream;
};

} // namespace Ota
