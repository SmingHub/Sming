/**
 * Since big portions of the code are copied from Finder.h and Finder.cpp are copied
 * from MrDunk's mDNS code these files are distributed under the same license as his project.
 *
 * MIT license: https://github.com/mrdunk/esp8266_mdns/blob/master/LICENCE.txt
 * 
 * References:
 * 
 *  Zero-configuration networking (DNS-SD) https://en.wikipedia.org/wiki/Zero-configuration_networking
 * 	Multicast DNS https://tools.ietf.org/html/rfc6762
 * 	DNS-Based Service Discovery https://tools.ietf.org/html/rfc6763
 * 	DNS record types https://en.wikipedia.org/wiki/List_of_DNS_record_types
 * 
 */
#pragma once

#include <Delegate.h>
#include <Network/UdpConnection.h>
#include "Query.h"
#include "Response.h"

namespace mDNS
{
// The mDNS spec says this should never be more than 256 (including trailing '\0').
static constexpr size_t MAX_MDNS_NAME_LEN{256};

class Finder : protected UdpConnection
{
public:
	using AnswerDelegate = Delegate<void(Response& response)>;

	Finder() : out(*this)
	{
	}

	~Finder();

	void onAnswer(AnswerDelegate callback)
	{
		answerCallback = callback;
	}

	bool search(const String& hostname, ResourceType type = ResourceType::SRV);

	bool search(const Query& query);

protected:
	void onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort) override;

private:
	/*
	 * Need a separate UDP connection for sending requests
	 */
	class UdpOut : public UdpConnection
	{
	public:
		UdpOut(Finder& finder) : finder(finder)
		{
		}

	protected:
		void onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort) override;
		Finder& finder;
	};

	void initialise();

	AnswerDelegate answerCallback;
	UdpOut out;
	bool initialised{false};
};

} // namespace mDNS
