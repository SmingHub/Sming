/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SpiFlash.h
 *
 ****/
#pragma once

#include "Device.h"

namespace Storage
{
extern SpiFlash* spiFlash;

/**
 * @brief Main flash storage device
 */
class SpiFlash : public Device
{
public:
	String getName() const override;
	size_t getBlockSize() const override;
	size_t getSize() const override;

	Type getType() const override
	{
		return Type::flash;
	}

	uint32_t getId() const;

	bool read(uint32_t address, void* dst, size_t size) override;
	bool write(uint32_t address, const void* src, size_t size) override;
	bool erase_range(uint32_t address, size_t size) override;
};

} // namespace Storage
