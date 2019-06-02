/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * NetUtils.h
 *
 ****/

/** @defgroup   networking Networking
 *  @{
 */

#pragma once

struct pbuf;
class String;

class NetUtils
{
public:
	// Helpers
	static bool pbufIsStrEqual(const pbuf* buf, const char* compared, unsigned startPos);
	static int pbufFindChar(const pbuf* buf, char wtf, unsigned startPos = 0);
	static int pbufFindStr(const pbuf* buf, const char* wtf, unsigned startPos = 0);
	static char* pbufAllocateStrCopy(const pbuf* buf, unsigned startPos, unsigned length);
	static String pbufStrCopy(const pbuf* buf, unsigned startPos, unsigned length);

#ifdef FIX_NETWORK_ROUTING
	static bool FixNetworkRouting();
#else
	static bool FixNetworkRouting()
	{
		return true; // Should work on standard lwip
	}
#endif

	// Debug
	static void debugPrintTcpList();

private:
#ifdef FIX_NETWORK_ROUTING
	static bool ipClientRoutingFixed;
#endif
};

/** @} */
