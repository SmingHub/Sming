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
#include <Storage.h>
#include <Storage/SpiFlash.h>

namespace Ota
{
class UpgraderBase
{
public:
	static constexpr uint8_t SLOT_NONE{255};

	using Partition = Storage::Partition;

	virtual ~UpgraderBase()
	{
	}

	/**
	 * @brief Prepares a partition for an upgrade.
	 * 		  The preparation is bootloader and architecture dependant.
	 * @param partition
	 * @param size
	 *
	 * @retval bool
	 */
	virtual bool begin(Partition partition, size_t size = 0) = 0;

	/**
	 * @brief Writes chunk of data to the partition set in ``begin()``.
	 * @param buffer
	 * @param size
	 *
	 * @retval size_t actually written bytes
	 */
	virtual size_t write(const uint8_t* buffer, size_t size) = 0;

	/**
	 * @brief Finilizes the partition upgrade.
	 */
	virtual bool end() = 0;

	/**
	* @brief Aborts a partition upgrade
	*/
	virtual bool abort()
	{
		return false;
	}

	/**
	 * @brief Sets the default parition from where the application will be booted on next restart.
	 * @param partition
	 * @param save  if true the change is persisted on the flash, otherwise it will be valid only for the next boot
	 *
	 * @retval bool
	 */
	virtual bool setBootPartition(Partition partition, bool save = true) = 0;

	/**
	 * @brief Gets information about the parition that is set as the default one to boot.
	 * @note The returned parition can be different than the current running partition.
	 *
	 * @retval partition
	 */
	virtual Partition getBootPartition() = 0;

	/**
	 * @brief Gets information about the parition from which the current application is running.
	 * @note The returned parition can be different than the default boot partition.
	 *
	 * @retval partition
	 */
	virtual Partition getRunningPartition() = 0;

	/**
	 * @brief Gets the next bootable partition that can be used after successful OTA upgrade
	 * @param startFrom - optional
	 *
	 * @retval partition
	 */
	virtual Partition getNextBootPartition(Partition startFrom = {}) = 0;

	/**
	 * @brief Gets information about all bootable partitions.
	 *
	 * @retval Storage::Iterator
	 */
	Storage::Iterator getBootPartitions()
	{
		return Storage::findPartition(Partition::Type::app);
	}

	// utility functions

	/**
	 * @brief Gets slot number for a partition
	 * @param partition
	 *
	 * @retval uint8_t slot number
	 */
	uint8_t getSlot(Partition partition)
	{
		if(partition.type() != Partition::Type::app) {
			return SLOT_NONE;
		}

		using App = Partition::SubType::App;
		auto subtype = App(partition.subType());
		if(subtype < App::ota_min || subtype > App::ota_max) {
			return SLOT_NONE;
		}

		return uint8_t(subtype) - uint8_t(App::ota_min);
	}
};

} // namespace Ota
