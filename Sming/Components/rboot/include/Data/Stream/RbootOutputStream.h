/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RbootOutputStream.h
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 *
 *  Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Data/Stream/ReadWriteStream.h>
#include <rboot-api.h>
#include <Storage/Partition.h>

/**
 * @brief Write-only stream type used during rBoot firmware updates
 */
class RbootOutputStream : public ReadWriteStream
{
public:
	/**
	 * @brief Construct a stream using raw flash address/size
	 * @param startAddress the start address on the storage media
	 * @param maxLength the maximum allowed length of the rom. Use 0 if unlimited.
	 * @note This should be avoided, use partition where possible
	 */
	RbootOutputStream(uint32_t startAddress, size_t maxLength = 0) : startAddress(startAddress), maxLength(maxLength)
	{
	}

	/**
	 * @brief Construct a stream for the given partition
	 * @param partition
	 */
	RbootOutputStream(Storage::Partition partition) : startAddress(partition.address()), maxLength(partition.size())
	{
	}

	virtual ~RbootOutputStream()
	{
		close();
	}

	size_t write(const uint8_t* data, size_t size) override;

	StreamType getStreamType() const override
	{
		return eSST_File;
	}

	uint16_t readMemoryBlock(char*, int) override
	{
		return 0;
	}

	bool seek(int) override
	{
		return false;
	}

	int available() override
	{
		return written;
	}

	bool isFinished() override
	{
		return true;
	}

	virtual bool close();

	size_t getStartAddress() const
	{
		return startAddress;
	}

	size_t getMaxLength() const
	{
		return maxLength;
	}

protected:
	bool initialized{false};
	rboot_write_status rBootWriteStatus{};
	size_t written{0};		  // << the number of written bytes
	uint32_t startAddress{0}; // << the start address on the storage
	size_t maxLength{0};	  // << maximum allowed length

protected:
	virtual bool init();
};
