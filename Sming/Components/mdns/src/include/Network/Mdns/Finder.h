/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Finder.h
 *
 * Code originally based on MrDunk's mDNS code distributed under the MIT license.
 *
 * MIT license: https://github.com/mrdunk/esp8266_mdns/blob/master/LICENCE.txt
 *
 * However, code has been essentially rewritten so Sming LGPL v3 license applies.
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

/**
 * @brief Locates mDNS services by issuing queries
 */
class Finder : protected UdpConnection
{
public:
	/**
	 * @brief Callback to be invoked for each received response.
	 */
	using AnswerDelegate = Delegate<void(Response& response)>;

	Finder() : out(*this)
	{
	}

	~Finder();

	/**
	 * @brief Set callback to be invoked for each received response
	 *
	 * An mDNS-SD (Multicast-DNS Service Discovery) response contains related answer records.
	 * The full set of answer records is passed to the callback.
	 */
	void onAnswer(AnswerDelegate callback)
	{
		answerCallback = callback;
	}

	/**
	 * @brief Send a multicast query request
	 * @param hostname Name to find, e.g. "_googlecast._tcp.local"
	 * @param type 
	 * @retval bool false if parameters failed validation or UDP request could not be sent
	 */
	bool search(const String& hostname, ResourceType type = ResourceType::PTR);

	/**
	 * @brief Send a multicast query request
	 * @param query Parameters for query
	 * @retval bool false if parameters failed validation or UDP request could not be sent
	 */
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

	bool initialise();

	AnswerDelegate answerCallback;
	UdpOut out;
	bool initialised{false};
};

} // namespace mDNS
