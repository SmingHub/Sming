/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * PayloadParser.h
 *
 *  Created: 2021 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "PayloadParser.h"
#include <Data/Stream/RbootOutputStream.h>

namespace OtaUpgrade
{
namespace Mqtt
{
/**
 * @brief This parser allows the processing of firmware data that is directly stored
 * 		  to the flash memory using RbootOutputStream.
 */
class RbootPayloadParser : public PayloadParser
{
public:
	RbootPayloadParser(const Storage::Partition& part, size_t currentVersion)
		: PayloadParser(currentVersion), part(part)
	{
	}

	bool switchRom(const UpdateState& updateState) override;

	ReadWriteStream* getStorageStream(size_t storageSize) override;

private:
	Storage::Partition part;
};

} // namespace Mqtt
} // namespace OtaUpgrade
