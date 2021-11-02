/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StandardPayloadParser.cpp
 *
 *  Created: 2021 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "include/OtaUpgrade/Mqtt/StandardPayloadParser.h"
#include <Ota/UpgradeOutputStream.h>
#include <Ota/Manager.h>

namespace OtaUpgrade
{
namespace Mqtt
{
bool StandardPayloadParser::switchRom(const UpdateState& updateState)
{
	auto after = OtaManager.getNextBootPartition();
#if DEBUG_VERBOSE_LEVEL >= DBG
	auto before = OtaManager.getBootPartition();
	debug_d("Swapping from %s @ 0x%08x to %s @ 0x%08x.\r\n", before.name().c_str(), before.address(),
			after.name().c_str(), after.address());
#endif

	return OtaManager.setBootPartition(after);
}

ReadWriteStream* StandardPayloadParser::getStorageStream(size_t storageSize)
{
	if(storageSize > part.size()) {
		debug_e("The new rom is too big to fit!");
		return nullptr;
	}

	return new Ota::UpgradeOutputStream(part, storageSize);
}

} // namespace Mqtt
} // namespace OtaUpgrade
