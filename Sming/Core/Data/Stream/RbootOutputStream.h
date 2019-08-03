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

class RbootOutputStream : public ReadWriteStream
{
public:
	/**
	 * @param startAddress the start address on the storage media
	 * @param maxLength the maximum allowed length of the rom. Use 0 if unlimited.
	 */
	RbootOutputStream(uint32_t startAddress, size_t maxLength = 0) : startAddress(startAddress), maxLength(maxLength)
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

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		return 0;
	}

	bool seek(int len) override
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
	bool initialized = false;
	rboot_write_status rBootWriteStatus;
	size_t written = 0;		   // << the number of written bytes
	uint32_t startAddress = 0; // << the start address on the storage
	size_t maxLength = 0;	  // << maximum allowed length

protected:
	virtual bool init();
};

/** @deprecated Use `RbootOutputStream` */
typedef RbootOutputStream rBootOutputStream SMING_DEPRECATED;
