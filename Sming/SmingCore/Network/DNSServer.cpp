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
 *
 * Created on March 4, 2016
 */
#include "DNSServer.h"
#include "UdpConnection.h"
#include "NetUtils.h"
#include "../Wiring/WString.h"

DNSServer::DNSServer()
{

  _ttl = htonl(60);
  _errorReplyCode = DNSReplyCode::NonExistentDomain;
}

DNSServer::~DNSServer(){

}

bool DNSServer::start(const uint16_t &port, const String &domainName,
                     const IPAddress &resolvedIP)
{
  _port = port;
  _buffer = NULL;
  _domainName = domainName;
  _resolvedIP[0] = resolvedIP[0];
  _resolvedIP[1] = resolvedIP[1];
  _resolvedIP[2] = resolvedIP[2];
  _resolvedIP[3] = resolvedIP[3];
  downcaseAndRemoveWwwPrefix(_domainName);
  return listen(_port) == 1;
}

void DNSServer::stop() {
	close();
	free(_buffer);
	_buffer = NULL;
}

void DNSServer::setErrorReplyCode(const DNSReplyCode &replyCode)
{
  _errorReplyCode = replyCode;
}

void DNSServer::setTTL(const uint32_t &ttl)
{
  _ttl = htonl(ttl);
}

void DNSServer::downcaseAndRemoveWwwPrefix(String &domainName)
{
  domainName.toLowerCase();
  domainName.replace("www.", "");
}

bool DNSServer::requestIncludesOnlyOneQuestion()
{
  return ntohs(_dnsHeader->QDCount) == 1 &&
         _dnsHeader->ANCount == 0 &&
         _dnsHeader->NSCount == 0 &&
         _dnsHeader->ARCount == 0;
}

void DNSServer::onReceive(pbuf* buf, IPAddress remoteIP, uint16_t remotePort)
{


    if (_buffer != NULL) free(_buffer);
    _buffer = (char*)malloc(buf->tot_len * sizeof(char));
    if (_buffer == NULL) return;
    pbuf_copy_partial(buf, _buffer, buf->tot_len, 0);
    debugf("DNS REQ for %s from %s:%d", getDomainNameWithoutWwwPrefix().c_str(), remoteIP.toString().c_str(), remotePort);
    _dnsHeader = (DNSHeader*) _buffer;
	if (_dnsHeader->QR == DNS_QR_QUERY &&
	        _dnsHeader->OPCode == DNS_OPCODE_QUERY &&
	        requestIncludesOnlyOneQuestion() &&
	        (_domainName == "*" || getDomainNameWithoutWwwPrefix() == _domainName)
	       )
		{
			char response[buf->tot_len+16];
			int idx = buf->tot_len;
			_dnsHeader->QR = DNS_QR_RESPONSE;
			_dnsHeader->ANCount = _dnsHeader->QDCount;
			_dnsHeader->QDCount = _dnsHeader->QDCount;
			memcpy(response, _buffer, idx);
			//Set a pointer to the domain name in the question section
			response[idx] = 0xC0;
			response[idx+1] = 0x0C;

			//Type: "Host Address"
			response[idx+2] = 0x00;
			response[idx+3] = 0x01;

			//Set the response class to IN
			response[idx+4] = 0x00;
			response[idx+5] = 0x01;

			//TTL
			response[idx+6] = _ttl >> 24;
			response[idx+7] = _ttl >> 16;
			response[idx+8] = _ttl >> 8;
			response[idx+9] = _ttl;

			//RDATA length
			response[idx+10] = 0x00;
			response[idx+11] = 0x04; //4 byte IP address

			//The IP address
			response[idx+12] = _resolvedIP[0];
			response[idx+13] = _resolvedIP[1];
			response[idx+14] = _resolvedIP[2];
			response[idx+15] = _resolvedIP[3];

			sendTo(remoteIP, remotePort, response, idx+16);
	    }
	    else if (_dnsHeader->QR == DNS_QR_QUERY)
	    {
	    	_dnsHeader->QR = DNS_QR_RESPONSE;
	    	_dnsHeader->RCode = (char) _errorReplyCode;
	    	_dnsHeader->QDCount = 0;
	    	sendTo(remoteIP, remotePort, _buffer, sizeof(DNSHeader));
	    }
	free(_buffer);
	_buffer = NULL;
	UdpConnection::onReceive(buf, remoteIP, remotePort);

}

String DNSServer::getDomainNameWithoutWwwPrefix()
{
	String parsedDomainName = "";
	if (_buffer == NULL) return parsedDomainName;
	char *start = _buffer + 12;
	if (*start == 0) return parsedDomainName;

	int pos = 0;
	while(true)
	{
		unsigned char labelLength = *(start + pos);
		for(int i = 0; i < labelLength; i++)
		{
			pos++;
			parsedDomainName += (char)*(start + pos);
		}
		pos++;
		if (*(start + pos) == 0)
		{
			downcaseAndRemoveWwwPrefix(parsedDomainName);
			return parsedDomainName;
		}
		else
		{
			parsedDomainName += ".";
		}
	}
}
