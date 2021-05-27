/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * UpgradeOutputStream.h
 *
 *
*/

#pragma once

#include <Ota/Upgrader.h>
#include <Storage/Partition.h>
#include <Data/Stream/ReadWriteStream.h>

namespace Ota
{
/**
 * @brief Write-only stream type used during firmware upgrade
 */
class UpgradeOutputStream : public ReadWriteStream
{
public:
	using Partition = Storage::Partition;

	/**
	 * @brief Construct a stream for the given partition
	 * @param partition
	 */
	UpgradeOutputStream(Partition partition, size_t maxLength = 0)
		: partition(partition), maxLength(maxLength != 0 ? std::min(maxLength, partition.size()) : partition.size())
	{
	}

	virtual ~UpgradeOutputStream()
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
		return partition.address();
	}

	size_t getMaxLength() const
	{
		return maxLength;
	}

protected:
	OtaUpgrader ota;
	Partition partition;
	bool initialized{false};
	size_t written{0};   // << the number of written bytes
	size_t maxLength{0}; // << maximum allowed length

protected:
	virtual bool init();
};

} // namespace Ota
