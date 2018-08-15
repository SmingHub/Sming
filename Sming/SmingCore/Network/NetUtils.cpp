/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "NetUtils.h"

#include "WString.h"
#ifdef __linux__
#include "lwip/priv/tcp_priv.h"
#else
#include "lwip/tcp_impl.h"
#endif

#ifdef FIX_NETWORK_ROUTING
bool NetUtils::_ipClientRoutingFixed = false;
#endif

/// HELPERS ///

int NetUtils::pbufFindChar(pbuf* buf, char wtf, unsigned startPos)
{
	if (!buf)
		return -1;

	int ofs = 0;

	while (buf->len <= startPos) {
		ofs += buf->len;
		startPos -= buf->len;
		buf = buf->next;
		if (!buf)
			return -1;
	}

	do {
		for (unsigned i = startPos; i < buf->len; i++) {
			char* sbuf = (char*)buf->payload;
			if (sbuf[i] == wtf) {
				//debug_d("%d %d", ofs, i);
				return ofs + i;
			}
		}
		ofs += buf->len;
		buf = buf->next;
		startPos = 0;
	} while (buf);

	return -1;
}

bool NetUtils::pbufIsStrEqual(pbuf* buf, const char* compared, unsigned startPos)
{
	if (!buf)
		return false;

	unsigned cur = startPos;

	while (buf->len <= cur) {
		cur -= buf->len;
		buf = buf->next;
		if (!buf)
			return false;
	}

	for (const char* cmp = compared; *cmp; cmp++) {
		if (!buf)
			return false;

		char& target = static_cast<char*>(buf->payload)[cur];
		if (target != *cmp)
			return false;

		cur++;
		if (cur >= buf->len) {
			cur = 0;
			buf = buf->next;
		}
	}

	return true;
}

int NetUtils::pbufFindStr(pbuf* buf, const char* wtf, unsigned startPos)
{
	if (!buf || !wtf)
		return -1;

	int cur = (int)startPos;
	while (true) {
		cur = pbufFindChar(buf, wtf[0], cur);
		if (cur < 0 || pbufIsStrEqual(buf, wtf, cur))
			return cur;

		cur++;
	}

	return -1;
}

char* NetUtils::pbufAllocateStrCopy(pbuf* buf, unsigned startPos, unsigned length)
{
	char* stringPtr = new char[length + 1];
	stringPtr[length] = '\0';
	pbuf_copy_partial(buf, stringPtr, length, startPos);
	return stringPtr;
}

String NetUtils::pbufStrCopy(pbuf* buf, unsigned startPos, unsigned length)
{
	char* stringPtr = new char[length + 1];
	stringPtr[length] = '\0';
	pbuf_copy_partial(buf, stringPtr, length, startPos);
	String res = String(stringPtr);
	delete[] stringPtr;
	return res;
}

bool NetUtils::FixNetworkRouting()
{
/*
 * @todo Is this code staying? Is it required for certain versions of LWIP ?
*/
#ifdef FIX_NETWORK_ROUTING
	if (ipClientRoutingFixed)
		return true;
	if (wifi_get_opmode() != STATIONAP_MODE)
		return true;

	ip_info info;
	wifi_get_ip_info(STATION_IF, &info);

	if (info.ip.addr == 0)
		return false;

	for (netif* netif = netif_list; netif; netif = netif->next)
		if (netif->ip_addr.addr == info.ip.addr) {
			debug_d("Fixed default network interface: %d.%d.%d.%d", IP2STR(&info.ip));
			netif_default = netif;
			ipClientRoutingFixed = true;
			return true;
		}

	return false;
#else
	// Should work on standard lwip
	return true;
#endif
}

/////////////////////////////////

// enum tcp_state, see file /include/lwip/tcp.h
static String tcp_state_str(tcp_state state)
{
#define XX(_n)                                                                                                         \
	case _n:                                                                                                           \
		return F(#_n)
	switch (state) {
		XX(CLOSED);
		XX(LISTEN);
		XX(SYN_SENT);
		XX(SYN_RCVD);
		XX(ESTABLISHED);
		XX(FIN_WAIT_1);
		XX(FIN_WAIT_2);
		XX(CLOSE_WAIT);
		XX(CLOSING);
		XX(LAST_ACK);
		XX(TIME_WAIT);
	default:
		return F("<Unknown>");
	}
}

static void debugPrintTcp(const struct tcp_pcb* pcb)
{
	unsigned active = 0;
	for (; pcb; pcb = pcb->next) {
		debugf("LWIP_DEBUG: Port %u | %u flg:%02x tmr:%04x %s\r\n", pcb->local_port, pcb->remote_port, pcb->flags,
			   pcb->tmr, tcp_state_str(pcb->state).c_str());
		active++;
	}

	if (active == 0)
		debugf("LWIP_DEBUG: none\r\n");
}

void NetUtils::debugPrintTcpList()
{
	debugf("********** Lwip Active PCB states:\r\n");
	debugPrintTcp(tcp_active_pcbs);

	debugf("********** Lwip Listen PCB states:\r\n");
	debugPrintTcp(tcp_listen_pcbs.pcbs);

	debugf("********** Lwip TIME-WAIT PCB states:\r\n");
	debugPrintTcp(tcp_tw_pcbs);
}
