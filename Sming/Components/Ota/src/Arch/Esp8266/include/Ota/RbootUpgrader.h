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
#include <rboot-api.h>

namespace Ota
{
class RbootUpgrader : public UpgraderBase
{
public:
	/**
	 * @brief Prepare the partition for
	 */
	bool begin(Partition partition, size_t size = 0) override;
	size_t write(const uint8_t* buffer, size_t size) override;

	bool end() override
	{
		return rboot_write_end(&status);
	}

	bool setBootPartition(Partition partition) override
	{
		return rboot_set_current_rom(getSlotForPartition(partition));
	}

	Partition getBootPartition() override
	{
		return getPartitionForSlot(rboot_get_current_rom());
	}

	Partition getRunningPartition() override;

	Partition getNextBootPartition(Partition startFrom = {}) override
	{
		uint8_t currentSlot = rboot_get_current_rom();
		if(startFrom) {
			currentSlot = getSlotForPartition(startFrom);
		}
		return getPartitionForSlot(currentSlot ? 0 : 1);
	}

	static uint8_t getSlotForPartition(Partition partition)
	{
		return (partition.subType() == uint8_t(Partition::SubType::App::ota1)) ? 1 : 0;
	}

	static Partition getPartitionForSlot(uint8_t slot)
	{
		return Storage::spiFlash->partitions().findOta(slot);
	}

private:
	rboot_write_status status{};
	size_t maxSize{0};
	size_t writtenSoFar{0};
};

} // namespace Ota
