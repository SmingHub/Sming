/* This file is part of Sming Framework Project
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * This file is provided under the LGPL v3 license.
 *
 * DnsServer.cpp
 *
 * File Author: https://github.com/patrickjahns
 *
 * The code is a port of the following projects
 * https://github.com/israellot/esp-ginx/tree/master/app/dns
 * https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer
 *
 * Created on March 4, 2016
 *
 ****/

#include "DNSServer.h"
#include "UdpConnection.h"
#include "WString.h"

bool DNSServer::start(uint16_t port, const String& domainName, const IpAddress& resolvedIP)
{
	this->port = port;
	buffer = nullptr;
	this->domainName = domainName;
	this->resolvedIP = resolvedIP;
	downcaseAndRemoveWwwPrefix(this->domainName);
	return listen(this->port) == 1;
}

void DNSServer::stop()
{
	close();
	delete[] buffer;
	buffer = nullptr;
}

void DNSServer::downcaseAndRemoveWwwPrefix(String& domainName)
{
	domainName.toLowerCase();
	domainName.replace(F("www."), String::empty);
}

bool DNSServer::requestIncludesOnlyOneQuestion()
{
	return ntohs(dnsHeader->QDCount) == 1 && dnsHeader->ANCount == 0 && dnsHeader->NSCount == 0 &&
		   dnsHeader->ARCount == 0;
}

void DNSServer::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
{
	delete[] buffer;
	buffer = new char[buf->tot_len];
	if(buffer == nullptr) {
		return;
	}

	pbuf_copy_partial(buf, buffer, buf->tot_len, 0);
	debug_d("DNS REQ for %s from %s:%d", getDomainNameWithoutWwwPrefix().c_str(), remoteIP.toString().c_str(),
			remotePort);
	dnsHeader = reinterpret_cast<DNSHeader*>(buffer);
	if(dnsHeader->QR == DNS_QR_QUERY && dnsHeader->OPCode == DNS_OPCODE_QUERY && requestIncludesOnlyOneQuestion() &&
	   (domainName == "*" || getDomainNameWithoutWwwPrefix() == domainName)) {
		char response[buf->tot_len + 16];
		int idx = buf->tot_len;
		dnsHeader->QR = DNS_QR_RESPONSE;
		dnsHeader->ANCount = dnsHeader->QDCount;
		memcpy(response, buffer, idx);
		//Set a pointer to the domain name in the question section
		response[idx] = 0xC0;
		response[idx + 1] = 0x0C;

		//Type: "Host Address"
		response[idx + 2] = 0x00;
		response[idx + 3] = 0x01;

		//Set the response class to IN
		response[idx + 4] = 0x00;
		response[idx + 5] = 0x01;

		//TTL
		auto ttln = htonl(ttl);
		response[idx + 6] = ttln >> 24;
		response[idx + 7] = ttln >> 16;
		response[idx + 8] = ttln >> 8;
		response[idx + 9] = ttln;

		//RDATA length
		response[idx + 10] = 0x00;
		response[idx + 11] = 0x04; //4 byte IP address

		//The IP address
		response[idx + 12] = ip4_addr1(&resolvedIP);
		response[idx + 13] = ip4_addr2(&resolvedIP);
		response[idx + 14] = ip4_addr3(&resolvedIP);
		response[idx + 15] = ip4_addr4(&resolvedIP);

		sendTo(remoteIP, remotePort, response, idx + 16);
	} else if(dnsHeader->QR == DNS_QR_QUERY) {
		dnsHeader->QR = DNS_QR_RESPONSE;
		dnsHeader->RCode = char(errorReplyCode);
		dnsHeader->QDCount = 0;
		sendTo(remoteIP, remotePort, buffer, sizeof(DNSHeader));
	}

	delete[] buffer;
	buffer = nullptr;

	UdpConnection::onReceive(buf, remoteIP, remotePort);
}

String DNSServer::getDomainNameWithoutWwwPrefix()
{
	String parsedDomainName;
	if(buffer == nullptr) {
		return parsedDomainName;
	}
	char* start = buffer + 12;
	if(*start == 0) {
		return parsedDomainName;
	}

	unsigned pos = 0;
	while(true) {
		unsigned labelLength = *(start + pos);
		for(unsigned i = 0; i < labelLength; i++) {
			pos++;
			parsedDomainName += start[pos];
		}
		pos++;
		if(start[pos] == 0) {
			downcaseAndRemoveWwwPrefix(parsedDomainName);
			return parsedDomainName;
		} else {
			parsedDomainName += '.';
		}
	}
}
