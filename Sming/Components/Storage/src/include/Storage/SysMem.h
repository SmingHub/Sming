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

#include "CustomDevice.h"

namespace Storage
{
/**
 * @brief Storage device to access system memory, e.g. RAM
 */
class SysMem : public CustomDevice
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

	size_t getSize() const override
	{
		return 0x80000000;
	}

	Type getType() const override
	{
		return Type::sysmem;
	}

	bool read(uint32_t address, void* buffer, size_t len) override
	{
		if(isFlashPtr(reinterpret_cast<const void*>(address))) {
			memcpy_P(buffer, reinterpret_cast<const void*>(address), len);
		} else {
			memcpy(buffer, reinterpret_cast<const void*>(address), len);
		}
		return true;
	}

	bool write(uint32_t address, const void* data, size_t len) override
	{
		if(isFlashPtr(reinterpret_cast<const void*>(address))) {
			return false;
		}

		memcpy(reinterpret_cast<void*>(address), data, len);
		return true;
	}

	bool erase_range(uint32_t address, size_t len) override
	{
		if(isFlashPtr(reinterpret_cast<const void*>(address))) {
			return false;
		}

		memset(&address, 0xFF, len);
		return true;
	}

	using CustomDevice::createPartition;

	/**
	 * @brief Create partition for FlashString data access
	 */
	Partition createPartition(const String& name, const FSTR::ObjectBase& fstr, Partition::Type type, uint8_t subtype);

	template <typename T> Partition createPartition(const String& name, const FSTR::ObjectBase& fstr, T subType)
	{
		return createPartition(name, fstr, Partition::Type(T::partitionType), uint8_t(subType));
	}
};

extern SysMem sysMem;

} // namespace Storage
