/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DnsServer.h
 *
 * File Author: https://github.com/patrickjahns
 *
 * The code is a port of the following projects
 * https://github.com/israellot/esp-ginx/tree/master/app/dns
 * https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer
 * Created on March 4, 2016
 *
 ****/

/** @defgroup   dnsserver DNS server
 *  @brief      Provides DNS server
 *  @ingroup    udp
 *  @{
 */
#pragma once

#include "UdpConnection.h"
#include <WString.h>
#include <IpAddress.h>

#define DNS_QR_QUERY 0
#define DNS_QR_RESPONSE 1
#define DNS_OPCODE_QUERY 0

enum class DnsReplyCode {
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

struct DnsHeader {
	uint16_t ID;	  // identification number
	char RD : 1;	  // recursion desired
	char TC : 1;	  // truncated message
	char AA : 1;	  // authoritative answer
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

/**
 * @brief DNS server class
 */
class DnsServer : public UdpConnection
{
public:
	/**
	 * @brief Set error reply code
	 */
	void setErrorReplyCode(DnsReplyCode replyCode)
	{
		errorReplyCode = replyCode;
	}

	/**
	 * @brief Set message Time-To-Live in seconds
	 */
	void setTTL(uint32_t ttl)
	{
		this->ttl = ttl;
	}

	/**
	 * @brief Start the DNS server
	 * @param port
	 * @param domainName
	 * @param resolvedIP
	 * @retval bool true if successful, false if there are no sockets available.
	 */
	bool start(uint16_t port, const String& domainName, const IpAddress& resolvedIP);

	/**
	 * @brief Stop the DNS server
	 */
	void stop()
	{
		close();
	}

protected:
	void onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort) override;
	size_t processQuestion(char* buffer, size_t requestLen);

private:
	uint16_t port = 0;
	String domainName;
	IpAddress resolvedIP;
	uint32_t ttl = 60;
	DnsReplyCode errorReplyCode = DnsReplyCode::NonExistentDomain;
};

/** @} */
