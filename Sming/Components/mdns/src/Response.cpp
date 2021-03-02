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
	uint16_t questionsCount = pkt.read16();
	if(questionsCount > 0) {
		// we are interested only in responses.
		return false;
	}

	// Number of incoming answers.
	uint16_t answersCount = pkt.read16();

	// Number of incoming Name Server resource records.
	uint16_t nsCount = pkt.read16();

	// Number of incoming Additional resource records.
	uint16_t additionalCount = pkt.read16();

	// List of answers
	bool ok{true};
	for(uint16_t i = 0; i < (answersCount + nsCount + additionalCount); i++) {
		auto answer = new Answer(*this);
		add(answer);
		if(!answer->parse(pkt)) {
			ok = false;
			break;
		}
	}

	return ok;
}

} // namespace mDNS
