/* This file is part of Sming Framework Project
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * This file is provided under the LGPL v3 license.
 *
 * File Author: https://github.com/patrickjahns
 *
 * The code is a port of the following projects
 * https://github.com/israellot/esp-ginx/tree/master/app/dns
 * https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer
 * Created on March 4, 2016
 */

/** @defgroup   dnsserver DNS server
 *  @brief      Provides DNS server
 *  @ingroup    udp
 *  @{
 */
#ifndef DNSServer_h
#define DNSServer_h

#include "UdpConnection.h"
#include "WString.h"

#define DNS_QR_QUERY 0
#define DNS_QR_RESPONSE 1
#define DNS_OPCODE_QUERY 0

enum class DNSReplyCode {
	NoError = 0,
	FormError = 1,
	ServerFailure = 2,
	NonExistentDomain = 3,
	NotImplemented = 4,
	Refused = 5,
	YXDomain = 6,
	YXRRSet = 7,
	NXRRSet = 8
};

struct DNSHeader {
	uint16_t ID;	  // identification number
	char RD : 1;	  // recursion desired
	char TC : 1;	  // truncated message
	char AA : 1;	  // authoritive answer
	char OPCode : 4;  // message_type
	char QR : 1;	  // query/response flag
	char RCode : 4;   // response code
	char Z : 3;		  // its z! reserved
	char RA : 1;	  // recursion available
	uint16_t QDCount; // number of question entries
	uint16_t ANCount; // number of answer entries
	uint16_t NSCount; // number of authority entries
	uint16_t ARCount; // number of resource entries
};

class DNSServer : public UdpConnection
{
public:
	DNSServer()
	{
	}

	virtual ~DNSServer()
	{
	}

	void setErrorReplyCode(DNSReplyCode replyCode)
	{
		errorReplyCode = replyCode;
	}

	void setTTL(uint32_t ttl)
	{
		this->ttl = ttl;
	}

	// Returns true if successful, false if there are no sockets available
	bool start(uint16_t port, const String& domainName, const IPAddress& resolvedIP);

	// stops the DNS server
	void stop();

private:
	uint16_t port = 0;
	String domainName;
	ip_addr resolvedIP;
	char* buffer = nullptr;
	DNSHeader* dnsHeader = nullptr;
	uint32_t ttl = 60;
	DNSReplyCode errorReplyCode = DNSReplyCode::NonExistentDomain;

	virtual void onReceive(pbuf* buf, IPAddress remoteIP, uint16_t remotePort);
	static void downcaseAndRemoveWwwPrefix(String& domainName);
	String getDomainNameWithoutWwwPrefix();
	bool requestIncludesOnlyOneQuestion();
};

/** @} */
#endif //DNSServer_h
