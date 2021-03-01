/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Channel.h
 *
 ****/

#pragma once

#include "../../../proto/cast_channel.pb.h"
#include <Protobuf.h>
#include <ArduinoJson.h>
#include <Data/LinkedObjectList.h>

namespace GoogleCast
{
DECLARE_FSTR(STANDARD_SENDER)
DECLARE_FSTR(STANDARD_RECEIVER)

/*
 *
 * See:
 * 	https://developers.google.com/cast/docs/reference/messages
 * 	https://github.com/thibauts/node-castv2
 * 	https://github.com/home-assistant-libs/pychromecast/tree/master/pychromecast/controllers
 * 
 */

class Client;

class Channel : public LinkedObjectTemplate<Channel>
{
public:
	using List = LinkedObjectListTemplate<Channel>;
	using OwnedList = OwnedLinkedObjectListTemplate<Channel>;

	class Message
	{
	public:
		using Delegate = ::Delegate<bool(Channel& channel, Message& message)>;

		extensions_api_cast_channel_CastMessage message;
		Protobuf::InputCallback source_id;
		Protobuf::InputCallback destination_id;
		Protobuf::InputCallback nameSpace;
		Protobuf::InputCallback payload_utf8;
		Protobuf::InputCallback payload_binary;

		enum class PayloadType {
			string = extensions_api_cast_channel_CastMessage_PayloadType_STRING,
			binary = extensions_api_cast_channel_CastMessage_PayloadType_BINARY,
		};

		Message()
			: message(extensions_api_cast_channel_CastMessage_init_default), source_id(message.source_id),
			  destination_id(message.destination_id), nameSpace(message.nameSpace), payload_utf8(message.payload_utf8),
			  payload_binary(message.payload_binary)
		{
		}

		PayloadType payloadType() const
		{
			return PayloadType(message.payload_type);
		}

		bool decode(Protobuf::InputStream& input)
		{
			return input.decode(extensions_api_cast_channel_CastMessage_fields, &message);
		}

		/**
		 * @brief De-serialize text payload in-situ (i.e. modifies data)
		 * @param doc Where to construct document
		 * @retval bool true on success
		 */
		bool deserialize(JsonDocument& doc)
		{
			if(payloadType() != PayloadType::string) {
				return false;
			}

			return Json::deserialize(doc, payload_utf8.getData(), payload_utf8.getLength());
		}
	};

	Channel(Client& client);

	virtual ~Channel();

	Client& getClient()
	{
		return client;
	}

	virtual String getNameSpace() const = 0;

	void setSourceId(const String& value)
	{
		sourceId = value;
	}

	String getSourceId() const;

	void setDestinationId(const String& value)
	{
		destinationId = value;
	}

	String getDestinationId() const;

	void onMessage(Message::Delegate handler)
	{
		callback = handler;
	}

	/**
	 * @name Send a message
	 * @param data
	 * @retval bool
	 * @{
	 */
	bool send(const String& data)
	{
		return send(reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
	}

	bool send(const char* data)
	{
		return send(data, strlen(data));
	}

	bool send(const void* data, size_t length);

	bool send(JsonDocument& json)
	{
		return send(Json::serialize(json));
	}

	/** @} */

	bool sendSimpleMessage(const String& type);

	/**
	 * @brief Handle a message for this namespace
	 * @param message
	 * @retval bool true if message was handled
	 * Return false to handle message via client callback.
	 */
	bool handleMessage(Message& message)
	{
		return callback ? callback(*this, message) : false;
	}

	void initRequest(JsonDocument& request, const String& type);

protected:
	Client& client;
	CString sourceId;
	CString destinationId;
	Message::Delegate callback;
};

} // namespace GoogleCast
