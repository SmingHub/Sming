/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AdvancedPayloadParser.h
 *
 *  Created: 2021 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "PayloadParser.h"
#include <OtaUpgradeStream.h>

namespace OtaUpgrade
{
namespace Mqtt
{
/**
 * This parser allows the processing of firmware data
 * that can be encrypted or have signature
 */
class AdvancedPayloadParser : public PayloadParser
{
public:
	using PayloadParser::PayloadParser;

	bool switchRom(const UpdateState& updateState) override;

	ReadWriteStream* getStorageStream(size_t storageSize) override;
};

} // namespace Mqtt
} // namespace OtaUpgrade
