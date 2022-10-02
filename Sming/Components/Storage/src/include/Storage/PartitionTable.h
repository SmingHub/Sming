/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * PartitionTable.h
 *
 ****/
#pragma once

#include "Partition.h"
#include "Iterator.h"

namespace Storage
{
// Used Partition table entries cached in RAM, initialised on first request
class PartitionTable
{
public:
	PartitionTable(Device& device) : mDevice(device)
	{
	}

	explicit operator bool() const
	{
		return mEntries.isEmpty();
	}

	/**
	 * @name Partition search
	 * @{
	 */

	/**
	 *
	 * @brief Find partitions based on one or more parameters
	 * @param type Partition type
	 * @param subtype Partition sub-type
	 * @retval Iterator Forward-iterator for matching partitions
	 */
	Iterator find(Partition::Type type = Partition::Type::any, uint8_t subType = Partition::SubType::any) const
	{
		return Iterator(mDevice, type, subType);
	}

	/// C++ subtype definition provides partition type
	template <typename T> Iterator find(T subType) const
	{
		return find(Partition::Type(T::partitionType), uint8_t(subType));
	}

	/** @} */

	/**
	 * @brief Find partition by name
	 * @param Name Name to search for, case-sensitive
	 * @retval Partition
	 *
	 * Names are unique so at most only one match
	 */
	Partition find(const String& name) const
	{
		return *std::find(begin(), end(), name);
	}

	/**
	 * @brief Find partition containing the given address
	 * @param address Address to search for
	 * @retval Partition
	 */
	Partition find(uint32_t address) const
	{
		return *std::find_if(begin(), end(), [address](Partition part) { return part.contains(address); });
	}

	/**
	 * @brief Find the n'th OTA partition
	 */
	Partition findOta(uint8_t index) const
	{
		using App = Partition::SubType::App;
		auto subtype = App(uint8_t(App::ota0) + index);
		return (subtype >= App::ota_min && subtype <= App::ota_max) ? *find(subtype) : Partition{};
	}

	Iterator begin() const
	{
		return Iterator(mDevice);
	}

	Iterator end() const
	{
		return Iterator(mDevice).end();
	}

	Device& device() const
	{
		return mDevice;
	}

protected:
	friend Device;
	friend Iterator;
	void load(const esp_partition_info_t* entry, unsigned count);

	Device& mDevice;
	Partition::Info::OwnedList mEntries;
};

} // namespace Storage
