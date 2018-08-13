/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/** @defgroup   networking Networking
 *  @{
 */

#ifndef _SMING_CORE_NETWORK_NETUTILS_H_
#define _SMING_CORE_NETWORK_NETUTILS_H_

struct pbuf;
class String;
class TcpConnection;

struct DnsLookup
{
	TcpConnection* con;
	int port;
};

class NetUtils {
public:
	// Helpers
	static bool pbufIsStrEqual(pbuf* buf, const char* compared, unsigned startPos);
	static int pbufFindChar(pbuf* buf, char wtf, unsigned startPos = 0);
	static int pbufFindStr(pbuf* buf, const char* wtf, unsigned startPos = 0);
	static char* pbufAllocateStrCopy(pbuf* buf, unsigned startPos, unsigned length);
	static String pbufStrCopy(pbuf* buf, unsigned startPos, unsigned length);

	static bool FixNetworkRouting();

	// Debug
	static void debugPrintTcpList();

private:
#ifdef FIX_NETWORK_ROUTING
	static bool _ipClientRoutingFixed;
#endif
};

/** @} */
#endif /* _SMING_CORE_NETWORK_NETUTILS_H_ */
