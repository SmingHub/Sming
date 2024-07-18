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
/**
 * @brief The main SPI flash device.
 * Where possible, access via partition API.
 * If lower-level access is required, use this.
 * It should not be necessary to access low-level flash access routines directly.
 */
extern SpiFlash* spiFlash;

/**
 * @brief Main flash storage device
 */
class SpiFlash : public Device
{
public:
	String getName() const override;
	size_t getBlockSize() const override;
	storage_size_t getSize() const override;

	Type getType() const override
	{
		return Type::flash;
	}

	uint32_t getId() const override;

	bool read(storage_size_t address, void* dst, size_t size) override;
	bool write(storage_size_t address, const void* src, size_t size) override;
	bool erase_range(storage_size_t address, storage_size_t size) override;
};

} // namespace Storage
