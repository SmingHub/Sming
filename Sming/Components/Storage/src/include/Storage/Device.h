/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Device.h - external storage device API
 *
 ****/
#pragma once

#include <WString.h>
#include <Printable.h>
#include <Data/LinkedObjectList.h>
#include "PartitionTable.h"

#define STORAGE_TYPE_MAP(XX)                                                                                           \
	XX(unknown, 0x00, "Other storage device")                                                                          \
	XX(flash, 0x01, "SPI flash")                                                                                       \
	XX(spiram, 0x02, "SPI RAM")                                                                                        \
	XX(sdcard, 0x03, "SD Card")                                                                                        \
	XX(disk, 0x04, "Physical disk")                                                                                    \
	XX(file, 0x05, "Backing file on separate filesystem")                                                              \
	XX(sysmem, 0x06, "System Memory")

namespace Storage
{
class SpiFlash;

/**
 * @brief Represents a storage device (e.g. flash memory)
 */
class Device : public LinkedObjectTemplate<Device>
{
public:
	using List = LinkedObjectListTemplate<Device>;
	using OwnedList = OwnedLinkedObjectListTemplate<Device>;

	/**
	 * @brief Storage type
	 */
	enum class Type : uint8_t {
#define XX(type, value, desc) type = value,
		STORAGE_TYPE_MAP(XX)
#undef XX
	};

	Device() : mPartitions(*this)
	{
	}

	~Device();

	bool operator==(const String& name) const
	{
		return getName() == name;
	}

	/**
	 * @brief Provide read-only access to partition table
	 */
	const PartitionTable& partitions() const
	{
		return mPartitions;
	}

	/**
	 * @brief Provide full access to partition table
	 */
	PartitionTable& editablePartitions()
	{
		return mPartitions;
	}

	/**
	 * @brief Load partition table entries
	 * @tableOffset Location of partition table to read
	 * @retval bool true on success, false on failure
	 */
	bool loadPartitions(uint32_t tableOffset)
	{
		return loadPartitions(*this, tableOffset);
	}

	/**
	 * @brief Load partition table entries from another table
	 * @param source Device to load entries from
	 * @tableOffset Location of partition table to read
	 * @retval bool true on success, false on failure
	 */
	bool loadPartitions(Device& source, uint32_t tableOffset);

	/**
	 * @brief Obtain unique device name
	 */
	virtual String getName() const = 0;

	/**
	 * @brief Obtain device ID
	 * @retval uint32_t typically flash chip ID
	 */
	virtual uint32_t getId() const
	{
		return 0;
	}

	/**
	 * @brief Obtain smallest allocation unit for erase operations
	 */
	virtual size_t getBlockSize() const = 0;

	/**
	 * @brief Obtain addressable size of this device
	 * @retval storage_size_t Must be at least as large as the value declared in the hardware configuration
	 */
	virtual storage_size_t getSize() const = 0;

	/**
	 * @brief Obtain device type
	 */
	virtual Type getType() const = 0;

	/**
	 * @brief Read data from the storage device
	 * @param address Where to start reading
	 * @param dst Buffer to store data
	 * @param size Size of data to be read, in bytes.
	 * @retval bool true on success, false on error
	 */
	virtual bool read(storage_size_t address, void* dst, size_t size) = 0;

	/**
	 * @brief Write data to the storage device
	 * @param address Where to start writing
	 * @param src Data to write
	 * @param size Size of data to be written, in bytes.
	 * @retval bool true on success, false on error
	 */
	virtual bool write(storage_size_t address, const void* src, size_t size) = 0;

	/**
	 * @brief Erase a region of storage in preparation for writing
	 * @param address Where to start erasing
	 * @param size Size of region to erase, in bytes
	 * @retval bool true on success, false on error
	 */
	virtual bool erase_range(storage_size_t address, storage_size_t size) = 0;

	size_t printTo(Print& p) const;

protected:
	PartitionTable mPartitions;
};

} // namespace Storage

String toString(Storage::Device::Type type);
String toLongString(Storage::Device::Type type);
