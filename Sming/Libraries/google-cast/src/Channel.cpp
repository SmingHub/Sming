/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Channel.cpp
 *
 ****/

#include "Network/GoogleCast/Client.h"
#include <Protobuf.h>

namespace GoogleCast
{
DEFINE_FSTR(STANDARD_SENDER, "sender-0")
DEFINE_FSTR(STANDARD_RECEIVER, "receiver-0")

Channel::Channel(Client& client) : client(client)
{
	client.channels.add(this);
}

Channel::~Channel()
{
	client.channels.remove(this);
}

String Channel::getSourceId() const
{
	String s(sourceId);
	return s ?: STANDARD_SENDER;
}

String Channel::getDestinationId() const
{
	String s(destinationId);
	return s ?: STANDARD_RECEIVER;
}

void Channel::initRequest(JsonDocument& request, const String& type)
{
	request[F("requestId")] = client.getNextRequestId();
	request[F("type")] = type;
}

bool Channel::sendSimpleMessage(const String& type)
{
	String msg;
	msg.reserve(16 + type.length());
	msg = F("{\"type\":\"");
	msg += type;
	msg += "\"}";
	return send(msg);
}

bool Channel::send(const void* data, size_t length)
{
	return client.sendMessage(*this, data, length);
}

} // namespace GoogleCast
