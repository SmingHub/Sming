/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Request.h
 *
 ****/

#pragma once

#include "Message.h"

namespace mDNS
{
class Request : public Message
{
public:
	/**
	 * @brief Create a unicast message
	 */
	Request(IpAddress remoteIp, uint16_t remotePort, Type type);

	/**
	 * @brief Create a multicast message
	 */
	Request(Type type) : Request(MDNS_IP, MDNS_TARGET_PORT, type)
	{
	}

	Question* addQuestion(const String& name, ResourceType type = ResourceType::PTR, uint16_t qclass = 1,
						  bool unicast = false);

	Answer* createAnswer(const String& name, Resource::Type type, uint16_t rclass, bool flush, uint32_t ttl);
	Answer* createAnswer(const Name& name, Resource::Type type, uint16_t rclass, bool flush, uint32_t ttl);

	template <class Resource, class N, typename... ParamTypes> Resource addAnswer(const N& name, ParamTypes... params)
	{
		auto answer = createAnswer(name, Resource::type, 1, false, 120);
		Resource r(*answer);
		r.init(params...);
		return r;
	}

	Answer::Kind nextSection()
	{
		assert(kind < Answer::Kind::additional);
		kind = Answer::Kind(unsigned(kind) + 1);
		return kind;
	}

private:
	uint8_t buffer[MAX_PACKET_SIZE];
	Answer::Kind kind{Answer::Kind::answer};
};

} // namespace mDNS
