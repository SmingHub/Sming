/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Request.cpp
 *
 ****/

#include "include/Network/Mdns/Request.h"
#include "Packet.h"

namespace mDNS
{
Request::Request(Type type) : Message(MDNS_IP, MDNS_TARGET_PORT, buffer, 0)
{
	Packet pkt{data, 0};

	// The first two bytes are the transaction id and they are not used in MDNS
	pkt.write16(0);

	// 2 bytes for Flags and status code
	pkt.write16(type == Type::query ? 0x0000 : 0x8000);

	// 2 bytes for number of questions
	pkt.write16(0);

	// 2 bytes for number of Answer RRs
	pkt.write16(0);

	// 2 bytes for Authority PRs
	pkt.write16(0);

	// 2 bytes for Additional PRs
	pkt.write16(0);

	size = pkt.pos;
}

Question* Request::addQuestion(const String& name, ResourceType type, uint16_t qclass, bool unicast)
{
	auto question = new Question(*this);
	questions.add(question);
	Packet pkt{data, 4};
	pkt.write16(questions.count());
	size += question->init(size, name, type, qclass, unicast);
	return question;
}

Answer* Request::createAnswer(const String& name, Resource::Type type, uint16_t rclass, bool flush, uint32_t ttl)
{
	auto answer = new Answer(*this, kind);
	answers.add(answer);
	Packet pkt{data, uint16_t(6 + unsigned(kind) * 2)};
	pkt.write16(pkt.peek16() + 1);
	size += answer->init(size, name, type, rclass, flush, ttl);
	return answer;
}

Answer* Request::createAnswer(const Name& name, Resource::Type type, uint16_t rclass, bool flush, uint32_t ttl)
{
	auto answer = new Answer(*this, kind);
	answers.add(answer);
	Packet pkt{data, uint16_t(6 + unsigned(kind) * 2)};
	pkt.write16(pkt.peek16() + 1);
	size += answer->init(size, name, type, rclass, flush, ttl);
	return answer;
}

} // namespace mDNS