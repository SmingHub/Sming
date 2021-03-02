#include "include/Network/Mdns/Response.h"
#include "Packet.h"
#include <Data/HexString.h>
#include <debug_progmem.h>

namespace mDNS
{
bool Response::parse()
{
	// check if we have a response or a query
	if(!isAnswer()) {
		debug_d("Message is not a response. Ignoring.");
		return false;
	}

	// TODO: If it's truncated we can expect more data soon so we should wait for additional records before deciding whether to respond.
	// if(isTruncated())
	// {
	// }

	// Non zero Response code implies error.
	if(getResponseCode() != 0) {
		debug_w("Got errored MDNS answer");
		return false;
	}

	Packet pkt{data, 4};

	// Number of incoming queries.
	auto questionsCount = pkt.read16();
	if(questionsCount != 0) {
		// we are interested only in responses.
		return false;
	}

	// Number of incoming answers.
	auto answersCount = pkt.read16();

	// Number of incoming Name Server resource records.
	auto nsCount = pkt.read16();

	// Number of incoming Additional resource records.
	auto additionalCount = pkt.read16();

	// List of answers
	bool ok{true};
	auto recordCount = answersCount + nsCount + additionalCount;
	for(uint16_t i = 0; i < recordCount; i++) {
		auto answer = new Answer(*this);
		if(!answer->parse(pkt)) {
			delete answer;
			return false;
		}
		add(answer);
	}

	return ok;
}

Answer* Response::operator[](ResourceType type)
{
	for(auto& ans : *this) {
		if(ans.getType() == type) {
			return &ans;
		}
	}
	return nullptr;
}

} // namespace mDNS
