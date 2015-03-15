/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_NETUTILS_H_
#define _SMING_CORE_NETWORK_NETUTILS_H_

struct pbuf;
class String;
class TcpConnection;

struct DnsLookup
{
	TcpConnection *con;
	int port;
};

class NetUtils
{
public:
	// Helpers
	static bool pbufIsStrEqual(pbuf *buf, const char* compared, int startPos);
	static int pbufFindChar(pbuf *buf, char wtf, int startPos = 0);
	static int pbufFindStr(pbuf *buf, const char* wtf, int startPos = 0);
	static char* pbufAllocateStrCopy(pbuf *buf, int startPos, int length);
	static String pbufStrCopy(pbuf *buf, int startPos, int length);

	static bool FixNetworkRouting();

	// Debug
	void debugPrintTcpList();

private:
	static bool ipClientRoutingFixed;
};

#endif /* _SMING_CORE_NETWORK_NETUTILS_H_ */
