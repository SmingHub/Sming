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

#include <Platform/System.h>
#include <Data/Stream/ReadWriteStream.h>
#include <Network/Mqtt/MqttPayloadParser.h>

namespace OtaUpgrade
{
namespace Mqtt
{
constexpr int8_t VERSION_NOT_READY{-1};

constexpr int8_t ERROR_INVALID_MQTT_MESSAGE{-1};
constexpr int8_t ERROR_INVALID_PATCH_VERSION{-2};
constexpr int8_t ERROR_UNKNOWN_REASON{-10};

class PayloadParser
{
public:
	struct UpdateState {
		ReadWriteStream* stream{nullptr};
		bool started{false};
		size_t version{0};

		~UpdateState()
		{
			delete stream;
		}
	};

	/**
	 * @brief
	 * @param currentPatchVersion
	 * @param allowedVersionBytes - maximum allowed bytes to be used for describing a patch version
	 */
	PayloadParser(size_t currentPatchVersion, size_t allowedVersionBytes = 24)
		: currentPatchVersion(currentPatchVersion), allowedVersionBytes(allowedVersionBytes)
	{
	}

	virtual ~PayloadParser()
	{
	}

	/**
	 * @brief This method is responsible for switching the rom.
	 * 		  This method is NOT restarting the system. It will happen lated in the parse method
	 * @retval true if the switch was successful
	 */
	virtual bool switchRom(const UpdateState& updateState) = 0;

	/**
	 * @brief Creates new stream to store the firmware update
	 * @param size_t storageSize the requested storage size
	 *
	 * @retval ReadWriteStream*
	 */
	virtual ReadWriteStream* getStorageStream(size_t storageSize) = 0;

	/**
	 * @brief This method takes care to read the incoming MQTT message and pass it to the stream that
	 * 		  is responsoble for storing the data.
	 *
	 * @retval int
	 * 				0 when everything is ok
	 * 				<0 when an error has occurred
	 */
	int parse(MqttPayloadParserState& state, mqtt_message_t* message, const char* buffer, int length);

private:
	int getPatchVersion(const char* buffer, int length, size_t& offset, size_t versionStart = 0);

	size_t currentPatchVersion;
	size_t allowedVersionBytes;
};

} // namespace Mqtt
} // namespace OtaUpgrade
