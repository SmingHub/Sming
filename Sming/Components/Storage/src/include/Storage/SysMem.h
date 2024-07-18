/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SysMem.h
 *
 ****/

#pragma once

#include "Device.h"

namespace Storage
{
/**
 * @brief Storage device to access system memory, e.g. RAM
 */
class SysMem : public Device
{
public:
	String getName() const override
	{
		return F("SysMem");
	}

	size_t getBlockSize() const override
	{
		return sizeof(uint32_t);
	}

	storage_size_t getSize() const override
	{
		return 0x80000000;
	}

	Type getType() const override
	{
		return Type::sysmem;
	}

	bool read(storage_size_t address, void* buffer, size_t len) override
	{
		if(isFlashPtr(reinterpret_cast<const void*>(address))) {
			memcpy_P(buffer, reinterpret_cast<const void*>(address), len);
		} else {
			memcpy(buffer, reinterpret_cast<const void*>(address), len);
		}
		return true;
	}

	bool write(storage_size_t address, const void* data, size_t len) override
	{
		if(isFlashPtr(reinterpret_cast<const void*>(address))) {
			return false;
		}

		memcpy(reinterpret_cast<void*>(address), data, len);
		return true;
	}

	bool erase_range(storage_size_t address, storage_size_t len) override
	{
		if(isFlashPtr(reinterpret_cast<const void*>(address))) {
			return false;
		}

		memset(&address, 0xFF, len);
		return true;
	}

	class SysMemPartitionTable : public PartitionTable
	{
	public:
		using PartitionTable::add;

		/**
		 * @brief Add partition entry for FlashString data access
		 */
		Partition add(const String& name, const FSTR::ObjectBase& fstr, Partition::FullType type)
		{
			return PartitionTable::add(name, type, reinterpret_cast<storage_size_t>(fstr.data()), fstr.size(),
									   Partition::Flag::readOnly);
		}
	};

	SysMemPartitionTable& editablePartitions()
	{
		return static_cast<SysMemPartitionTable&>(mPartitions);
	}
};

extern SysMem sysMem;

} // namespace Storage
