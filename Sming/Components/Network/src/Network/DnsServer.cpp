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

#include "DnsServer.h"
#include <lwip_includes.h>
#include <debug_progmem.h>

#define MAX_ANSWER_LENGTH 16

namespace
{
void downcaseAndRemoveWwwPrefix(String& domainName)
{
	domainName.toLowerCase();
	if(domainName.startsWith(_F("www."))) {
		domainName.remove(0, 4);
	}
}

String getDomainNameWithoutWwwPrefix(const char* buffer, size_t buflen)
{
	unsigned srcpos = 12;
	if(buflen < 13 || buffer[srcpos] == 0) {
		return nullptr;
	}

	String name;
	name.reserve(buflen - 12);
	while(srcpos < buflen) {
		unsigned labelLength = buffer[srcpos++];
		if(srcpos + labelLength > buflen) {
			break;
		}
		name.concat(&buffer[srcpos], labelLength);
		srcpos += labelLength;
		if(buffer[srcpos] == 0) {
			downcaseAndRemoveWwwPrefix(name);
			return name;
		}
		name += '.';
	}

	return nullptr;
}

bool requestIncludesOnlyOneQuestion(const DnsHeader& dnsHeader)
{
	return ntohs(dnsHeader.QDCount) == 1 && dnsHeader.ANCount == 0 && dnsHeader.NSCount == 0 && dnsHeader.ARCount == 0;
}

} // namespace

bool DnsServer::start(uint16_t port, const String& domainName, const IpAddress& resolvedIP)
{
	this->port = port;
	this->domainName = domainName;
	this->resolvedIP = resolvedIP;
	downcaseAndRemoveWwwPrefix(this->domainName);
	return listen(this->port) == 1;
}

void DnsServer::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
{
	debug_d("DNS REQ from %s:%d", remoteIP.toString().c_str(), remotePort);

	// Allocated buffer with additional room for answer
	char* buffer = new char[buf->tot_len + MAX_ANSWER_LENGTH];
	if(buffer == nullptr) {
		return;
	}

	unsigned requestLen = pbuf_copy_partial(buf, buffer, buf->tot_len, 0);
	if(requestLen != buf->tot_len) {
		delete[] buffer;
		return;
	}

	debug_hex(DBG, "< DNS", buffer, requestLen);

	auto responseLen = processQuestion(buffer, requestLen);
	if(responseLen != 0) {
		debug_hex(DBG, "> DNS", buffer, responseLen);
		sendTo(remoteIP, remotePort, buffer, responseLen);
	}

	delete[] buffer;

	UdpConnection::onReceive(buf, remoteIP, remotePort);
}

size_t DnsServer::processQuestion(char* buffer, size_t requestLen)
{
	auto& dnsHeader = *reinterpret_cast<DnsHeader*>(buffer);
	if(dnsHeader.QR != DNS_QR_QUERY) {
		debug_d("DNS ignoring, not QUERY");
		return 0;
	}

	String parsedDomainName = getDomainNameWithoutWwwPrefix(buffer, requestLen);
	debug_d("DNS REQ for %s", parsedDomainName.c_str());

	if(dnsHeader.OPCode == DNS_OPCODE_QUERY && requestIncludesOnlyOneQuestion(dnsHeader) &&
	   (domainName == "*" || domainName == parsedDomainName)) {
		dnsHeader.QR = DNS_QR_RESPONSE;
		dnsHeader.ANCount = dnsHeader.QDCount;
		auto response = &buffer[requestLen];
		// Set a pointer to the domain name in the question section
		response[0] = 0xC0;
		response[1] = 0x0C;

		// Type: "Host Address"
		response[2] = 0x00;
		response[3] = 0x01;

		// Set the response class to IN
		response[4] = 0x00;
		response[5] = 0x01;

		// TTL
		auto ttln = htonl(ttl);
		response[6] = ttln >> 24;
		response[7] = ttln >> 16;
		response[8] = ttln >> 8;
		response[9] = ttln;

		// RDATA length
		response[10] = 0x00;
		response[11] = 0x04; //4 byte IP address

		// The IP address
		response[12] = resolvedIP[0];
		response[13] = resolvedIP[1];
		response[14] = resolvedIP[2];
		response[15] = resolvedIP[3];

		return requestLen + 16;
	}

	dnsHeader.QR = DNS_QR_RESPONSE;
	dnsHeader.RCode = char(errorReplyCode);
	dnsHeader.QDCount = 0;
	return sizeof(DnsHeader);
}
