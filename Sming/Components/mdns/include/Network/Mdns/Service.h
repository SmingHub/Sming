#pragma once

#include <user_config.h>
#include <Data/CStringArray.h>
#include <memory>

#if ENABLE_CUSTOM_LWIP == 2
#ifdef ARCH_ESP8266
#define MDNS_LWIP 0x0200
#else
#define MDNS_LWIP 0x0202
#endif
#elif ENABLE_CUSTOM_LWIP == 1
#define MDNS_LWIP 0x0101
#else
#define MDNS_LWIP 0x0100
#endif

namespace mDNS
{
class Responder;

class Service
{
public:
	enum class Protocol {
		Udp /* = DNSSD_PROTO_UCP */,
		Tcp /* = DNSSD_PROTO_TCP */,
	};

	/**
	 * @brief Basic service information
	 *
	 * Defaults fine for most cases, just need to set name.
	 */
	struct Info {
		String name = "Sming";
		String type = "http";
		Protocol protocol = Protocol::Tcp;
		uint16_t port = 80;
	};

	/**
	 * @brief Override to obtain service information
	 */
	virtual Info getInfo() = 0;

	/**
	 * @brief Override to obtain txt items
	 *
	 * LWIP2 calls this each time a TXT reply is created
	 * Other implementations call it via begin().
	 *
	 * @retval CStringArray List of txt items, e.g. name=value pairs
	 */
	virtual CStringArray getTxt()
	{
		return nullptr;
	}

private:
	friend class Responder;

#if MDNS_LWIP >= 0x0200
	int8_t id = -1;
#else
	String type;
	CStringArray txt;
#endif
};

} // namespace mDNS
