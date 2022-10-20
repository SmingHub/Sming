/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ProgMem.h
 *
 ****/
#pragma once

#include "Device.h"

namespace Storage
{
/**
 * @brief Storage device to access PROGMEM using flash API
 */
class ProgMem : public Device
{
public:
	String getName() const override
	{
		return F("ProgMem");
	}

	size_t getBlockSize() const override
	{
		return sizeof(uint32_t);
	}

	size_t getSize() const override
	{
		return 0x80000000;
	}

	Type getType() const override
	{
		return Type::flash;
	}

	bool read(uint32_t address, void* dst, size_t size) override;

	bool write(uint32_t address, const void* src, size_t size) override
	{
		return false;
	}

	bool erase_range(uint32_t address, size_t size) override
	{
		return false;
	}

	class ProgMemPartitionTable : public PartitionTable
	{
	public:
		/**
		 * @brief Add partition entry for PROGMEM data access
		 * @param name Name for partition
		 * @param flashPtr PROGMEM pointer
		 * @param size Size of PROGMEM data
		 * @param type Partition type and subtype
		 * @retval Partition Invalid if data is not progmem
		 */
		Partition add(const String& name, const void* flashPtr, size_t size, Partition::FullType type);

		/**
		 * @brief Add partition entry for FlashString data access
		 */
		Partition add(const String& name, const FSTR::ObjectBase& fstr, Partition::FullType type)
		{
			return add(name, fstr.data(), fstr.size(), type);
		}
	};

	ProgMemPartitionTable& editablePartitions()
	{
		return static_cast<ProgMemPartitionTable&>(mPartitions);
	}
};

extern ProgMem progMem;

} // namespace Storage
