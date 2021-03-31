/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * PayloadParser.cpp
 *
 *  Created: 2021 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "include/OtaUpgrade/Mqtt/PayloadParser.h"

namespace OtaUpgrade
{
namespace Mqtt
{
int PayloadParser::parse(MqttPayloadParserState& state, mqtt_message_t* message, const char* buffer, int length)
{
	if(message == nullptr) {
		debug_e("Invalid MQTT message");
		return ERROR_INVALID_MQTT_MESSAGE;
	}

	if(length == MQTT_PAYLOAD_PARSER_START) {
		state = MqttPayloadParserState{new UpdateState};
		return 0;
	}

	auto updateState = static_cast<UpdateState*>(state.userData);
	if(updateState == nullptr) {
		debug_e("Update failed for unknown reason!");
		return ERROR_UNKNOWN_REASON;
	}

	if(length == MQTT_PAYLOAD_PARSER_END) {
		bool skip = (updateState->stream == nullptr);
		if(!skip) {
			bool success = switchRom(*updateState);
			if(success) {
				debug_d("Switching was successful. Restarting...");
				System.restart();
			} else {
				debug_e("Switching failed!");
			}
		}
		delete updateState;
		state.userData = nullptr;
		return 0;
	}

	if(buffer == nullptr) {
		debug_e("Invalid MQTT message");
		return ERROR_INVALID_MQTT_MESSAGE;
	}

	if(!updateState->started) {
		size_t offset = 0;
		int patchVersion = getPatchVersion(buffer, length, offset, updateState->version);
		state.offset += offset;
#if ENABLE_OTA_VARINT_VERSION
		if(patchVersion < 0) {
			if(state.offset > allowedVersionBytes) {
				debug_e("Invalid patch version.");
				return ERROR_INVALID_PATCH_VERSION;
			}
			return 0;
		}
#endif

		updateState->started = true;
		if(size_t(patchVersion) < currentPatchVersion) {
			// The update is not newer than our current patch version
			return 0;
		}

		length -= offset;
		buffer += offset;

		updateState->stream = getStorageStream(message->common.length - offset);
	}

	auto stream = updateState->stream;
	if(stream == nullptr) {
		return 0;
	}

	auto written = stream->write(buffer, length);
	return (written - length);
}

int PayloadParser::getPatchVersion(const char* buffer, int length, size_t& offset, size_t versionStart)
{
	if(buffer == nullptr || length < 1) {
		return ERROR_INVALID_PATCH_VERSION;
	}

	size_t version = versionStart;
#if ENABLE_OTA_VARINT_VERSION
	offset = 0;
	int useNextByte = 0;
	do {
		version += (buffer[offset] & 0x7f);
		useNextByte = (buffer[offset++] & 0x80);
	} while(useNextByte && (offset < size_t(length)));

	if(useNextByte) {
		// all the data is consumed and we still don't have a version number?!
		return VERSION_NOT_READY;
	}
#else
	offset = 1;
	version += buffer[0];
#endif

	return version;
}

} // namespace Mqtt
} // namespace OtaUpgrade
