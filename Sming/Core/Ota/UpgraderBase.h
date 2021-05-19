/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Ota.h
 *
 * This header includes all unified Over-The-Air functions.
 *
*/

#pragma once
#include <Storage.h>

namespace Ota
{
class UpgraderBase
{
public:
	virtual ~UpgraderBase()
	{
	}

	/**
	 * @brief Prepare the partition for
	 */
	virtual bool begin(Storage::Partition partition, size_t size) = 0;
	virtual size_t write(const uint8_t* buffer, size_t size) = 0;
	virtual bool seek(int len)
	{
		return false;
	}

	virtual bool end() = 0;

	virtual bool abort()
	{
		return false;
	}

	virtual bool setBootPartition(Storage::Partition partition) = 0;
	virtual Storage::Partition getBootPartition(void) = 0;
	virtual Storage::Partition getRunningPartition(void) = 0;
	virtual Storage::Partition getNextUpdatePartition(Storage::Partition* startFrom = nullptr) = 0;
};

} // namespace Ota
