/**
 * Since big portions of the code are copied from Finder.h and Finder.cpp are copied
 * from MrDunk's mDNS code these files are distributed under the same license as his project.
 *
 * MIT license: https://github.com/mrdunk/esp8266_mdns/blob/master/LICENCE.txt
 */
#pragma once

#include <user_config.h>
#include <Delegate.h>
#include <WString.h>
#include <Network/UdpConnection.h>

namespace mDNS
{
#define MDNS_IP 224, 0, 0, 251
#define MDNS_TARGET_PORT 5353
#define MDNS_SOURCE_PORT 5353
#define MDNS_TTL 255

#define MAX_PACKET_SIZE 1024

// The mDNS spec says this should never be more than 256 (including trailing '\0').
#define MAX_MDNS_NAME_LEN 256

/**
 * @brief MDNS resource type identifiers
 * 
 * (name, value, description)
 */
#define MDNS_RESOURCE_TYPE_MAP(XX)                                                                                     \
	XX(A, 0x0001, "32-bit IPv4 address")                                                                               \
	XX(PTR, 0x000C, "Pointer to a canonical name")                                                                     \
	XX(HINFO, 0x000D, "Host Information")                                                                              \
	XX(TXT, 0x0010, "Arbitrary human-readable text")                                                                   \
	XX(AAAA, 0x001C, "128-bit IPv6 address")                                                                           \
	XX(SRV, 0x0021, "Server selection")

enum class ResourceType : uint16_t {
#define XX(name, value, desc) name = value,
	MDNS_RESOURCE_TYPE_MAP(XX)
#undef XX
};

/**
 * @brief A single mDNS Query
 */
struct Query {
	char name[MAX_MDNS_NAME_LEN]; ///< Question Name: Contains the object, domain or zone name.
	ResourceType type;			  ///< Question Type: Type of question being asked by client.
	uint16_t klass;				  ///< Question Class: Normally the value 1 for Internet (“IN”)
	bool isUnicastResponse;		  //
	bool isValid;				  ///< False if problems were encountered decoding packet.
};

/**
 * @brief A single mDNS Answer
 */
struct Answer {
	void* rawData;
	uint16_t rawDataLen;
	String name;		///< object, domain or zone name.
	String data;		///< The decoded data portion of the resource record.
	ResourceType type;  ///< ResourceRecord Type.
	uint16_t klass;		///< ResourceRecord Class: Normally the value 1 for Internet (“IN”)
	uint32_t ttl;		///< ResourceRecord Time To Live: Number of seconds ths should be remembered.
	bool isCachedFlush; ///< Flush cache of records matching this name.
	bool isValid;		///< False if problems were encountered decoding packet.
	// Decoded fields dependent upon resource type
	union {
		struct {
			uint32_t addr;
		} a;
		struct {
			uint16_t priority;
			uint16_t weight;
			uint16_t port;
		} srv;
	};
};

class Finder : protected UdpConnection
{
public:
	using AnswerDelegate = Delegate<void(Answer& answer)>;

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

String toString(mDNS::ResourceType type);
