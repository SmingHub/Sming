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

#include "include/OtaUpgrade/Mqtt/AdvancedPayloadParser.h"

namespace OtaUpgrade
{
namespace Mqtt
{
bool AdvancedPayloadParser::switchRom(const UpdateState& updateState)
{
	if(updateState.stream == nullptr) {
		return false;
	}

	auto otaStream = static_cast<OtaUpgradeStream*>(updateState.stream);
	if(otaStream == nullptr) {
		return false;
	}

	if(otaStream->hasError()) {
		debug_e("Got error: %s", toString(otaStream->errorCode).c_str());
		return false;
	}

	return true;
}

ReadWriteStream* AdvancedPayloadParser::getStorageStream(size_t storageSize)
{
	return new OtaUpgradeStream();
}

} // namespace Mqtt
} // namespace OtaUpgrade
