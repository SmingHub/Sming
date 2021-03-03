/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Response.cpp
 *
 ****/

#include "include/Network/Mdns/Response.h"
#include "Packet.h"
#include <Data/HexString.h>
#include <debug_progmem.h>

namespace mDNS
{
bool Response::parse()
{
	// TODO: If it's truncated we can expect more data soon so we should wait for additional records before deciding whether to respond.
	// if(isTruncated())
	// {
	// }

	// Non zero Response code implies error.
	if(getResponseCode() != 0) {
		debug_w("Got errored MDNS response");
		return false;
	}

	Packet pkt{data, 4};

	// Number of incoming queries.
	auto questionsCount = pkt.read16();

	// Number of incoming answers.
	auto answersCount = pkt.read16();

	// Number of incoming Name Server resource records.
	auto nsCount = pkt.read16();

	// Number of incoming Additional resource records.
	auto additionalCount = pkt.read16();

	// List of questions
	for(uint16_t i = 0; i < questionsCount; i++) {
		auto question = new Question(*this);
		if(!question->parse(pkt)) {
			delete question;
			return false;
		}
		questions.add(question);
	}

	// List of answers, namespaces and additional records
	bool ok{true};
	auto recordCount = answersCount + nsCount + additionalCount;
	for(uint16_t i = 0; i < recordCount; i++) {
		auto answer = new Answer(*this);
		if(!answer->parse(pkt)) {
			delete answer;
			return false;
		}
		answers.add(answer);
	}

	return ok;
}

Answer* Response::operator[](ResourceType type)
{
	for(auto& ans : answers) {
		if(ans.getType() == type) {
			return &ans;
		}
	}
	return nullptr;
}

} // namespace mDNS
