/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * NetUtils.h
 *
 ****/

#pragma once

#ifdef ARCH_ESP8266
#include "lwip/tcp_impl.h"
#else
#include "lwip/priv/tcp_priv.h"
#endif

struct pbuf;
class String;

/** @defgroup   networking Networking
 *  @{
 */

namespace NetUtils
{
// Helpers
bool pbufIsStrEqual(const pbuf* buf, const char* compared, unsigned startPos);
int pbufFindChar(const pbuf* buf, char wtf, unsigned startPos = 0);
int pbufFindStr(const pbuf* buf, const char* wtf, unsigned startPos = 0);
char* pbufAllocateStrCopy(const pbuf* buf, unsigned startPos, unsigned length);
String pbufStrCopy(const pbuf* buf, unsigned startPos, unsigned length);

#ifdef FIX_NETWORK_ROUTING
bool FixNetworkRouting();
#else
inline bool FixNetworkRouting()
{
	return true; // Should work on standard lwip
}
#endif

// Debug
void debugPrintTcpList();

}; // namespace NetUtils

/** @} */
