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

#include "include/OtaUpgrade/Mqtt/RbootPayloadParser.h"

namespace OtaUpgrade
{
namespace Mqtt
{
bool RbootPayloadParser::switchRom(const UpdateState& updateState)
{
	uint8 before, after;
	before = rboot_get_current_rom();
	if(before == 0) {
		after = 1;
	} else {
		after = 0;
	}
	debug_d("Swapping from rom %d to rom %d.\r\n", before, after);
	return rboot_set_current_rom(after);
}

ReadWriteStream* RbootPayloadParser::getStorageStream(size_t storageSize)
{
	if(storageSize > part.size()) {
		debug_e("The new rom is too big to fit!");
		return nullptr;
	}

	return new RbootOutputStream(part.address(), part.size());
}

} // namespace Mqtt
} // namespace OtaUpgrade
