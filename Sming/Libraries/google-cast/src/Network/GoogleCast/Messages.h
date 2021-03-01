#pragma once

#include "../../../proto/cast_channel.pb.h"
#include <Protobuf.h>

namespace GoogleCast
{
class ChannelMessage
{
public:
	using Delegate = ::Delegate<bool(ChannelMessage& message)>;

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

	ChannelMessage()
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
};

} // namespace GoogleCast