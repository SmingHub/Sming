#include "include/OtaUpgrade/Mqtt/PayloadParser.h"

namespace OtaUpgrade
{
namespace Mqtt
{
int PayloadParser::parse(MqttPayloadParserState& state, mqtt_message_t* message, const char* buffer, int length)
{
	if(message == nullptr) {
		debug_e("Invalid MQTT message");
		return 1;
	}

	if(length == MQTT_PAYLOAD_PARSER_START) {
		UpdateState* updateState = new UpdateState();
		updateState->stream = nullptr;
		updateState->started = false;

		state.offset = 0;
		state.userData = updateState;
		return 0;
	}

	auto updateState = static_cast<UpdateState*>(state.userData);
	if(updateState == nullptr) {
		debug_e("Update failed for unknown reason!");
		return -1;
	}

	if(length == MQTT_PAYLOAD_PARSER_END) {
		bool skip = (updateState->stream == nullptr);
		if(!skip) {
			bool success = switchRom(*updateState);
			delete updateState->stream;
			delete updateState;
			if(success) {
				debug_d("Swtiching was successful. Restarting...");
				System.restart();
			} else {
				debug_e("Swtiching failed!");
			}
		}

		return 0;
	}

	if(!updateState->started) {
		size_t offset = 0;
		int patchVersion = getPatchVersion(buffer, length, offset, updateState->version);
		state.offset += offset;
#if ENABLE_OTA_VARINT_VERSION
		if(currentPatchVersion < 0) {
			if(state.offset > VERSION_MAX_BYTES_ALLOWED) {
				debug_e("Invalid patch version.");
				return -3; //
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

	auto written = stream->write(reinterpret_cast<const uint8_t*>(buffer), length);
	return (written - length);
}

#if ENABLE_OTA_VARINT_VERSION
int PayloadParser::getPatchVersion(const char* buffer, int length, size_t& offset, size_t versionStart)
{
	size_t version = versionStart;
	offset = 0;
	int useNextByte = 0;
	do {
		version += (buffer[offset] & 0x7f);
		useNextByte = (buffer[offset++] & 0x80);
	} while(useNextByte && (offset < length));

	if(useNextByte) {
		// all the data is consumed and we still don't have a version number?!
		return VERSION_NOT_READY;
	}

	return version;
}
#else
int PayloadParser::getPatchVersion(const char* buffer, int length, size_t& offset, size_t versionStart)
{
	offset = 1;
	return buffer[0];
}
#endif

} // namespace Mqtt
} // namespace OtaUpgrade
