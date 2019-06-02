/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * NetUtils.cpp
 *
 ****/

#include "NetUtils.h"
#include "Data/CStringArray.h"

#include "WString.h"
#ifdef __ets__
#include "lwip/tcp_impl.h"
#else
#include "lwip/priv/tcp_priv.h"
#endif

#ifdef FIX_NETWORK_ROUTING
bool NetUtils::ipClientRoutingFixed = false;
#endif

/// HELPERS ///

int NetUtils::pbufFindChar(const pbuf* buf, char wtf, unsigned startPos)
{
	unsigned ofs = 0;

	while(buf->len <= startPos) {
		ofs += buf->len;
		startPos -= buf->len;
		buf = buf->next;
		if(buf == nullptr) {
			return -1;
		}
	}

	do {
		for(unsigned i = startPos; i < buf->len; i++) {
			char* sbuf = (char*)buf->payload;
			if(sbuf[i] == wtf) {
				//debug_d("%d %d", ofs, i);
				return ofs + i;
			}
		}
		ofs += buf->len;
		buf = buf->next;
		startPos = 0;
	} while(buf != nullptr);

	return -1;
}

bool NetUtils::pbufIsStrEqual(const pbuf* buf, const char* compared, unsigned startPos)
{
	unsigned cur = startPos;

	while(buf->len <= cur) {
		cur -= buf->len;
		buf = buf->next;
		if(buf == nullptr) {
			return false;
		}
	}

	for(const char* cmp = compared; *cmp; cmp++) {
		if(buf == nullptr)
			return false;

		char& target = ((char*)buf->payload)[cur];
		if(target != *cmp)
			return false;

		cur++;
		if(cur >= buf->len) {
			cur = 0;
			buf = buf->next;
		}
	}

	return true;
}

int NetUtils::pbufFindStr(const pbuf* buf, const char* wtf, unsigned startPos)
{
	if(wtf == nullptr) {
		return -1;
	}

	int cur = startPos;
	while(true) {
		cur = pbufFindChar(buf, wtf[0], cur);
		if(cur < 0) {
			return -1;
		}

		if(pbufIsStrEqual(buf, wtf, cur)) {
			return cur;
		}
		cur++;
	}

	return -1;
}

char* NetUtils::pbufAllocateStrCopy(const pbuf* buf, unsigned startPos, unsigned length)
{
	char* stringPtr = new char[length + 1];
	if(stringPtr == nullptr) {
		return nullptr;
	}
	stringPtr[length] = '\0';
	pbuf_copy_partial(const_cast<pbuf*>(buf), stringPtr, length, startPos);
	return stringPtr;
}

String NetUtils::pbufStrCopy(const pbuf* buf, unsigned startPos, unsigned length)
{
	char* stringPtr = new char[length + 1];
	if(stringPtr == nullptr) {
		return nullptr;
	}
	stringPtr[length] = '\0';
	pbuf_copy_partial(const_cast<pbuf*>(buf), stringPtr, length, startPos);
	String res = String(stringPtr);
	delete[] stringPtr;
	return res;
}

#ifdef FIX_NETWORK_ROUTING
bool NetUtils::FixNetworkRouting()
{
	if(ipClientRoutingFixed) {
		return true;
	}

	if(wifi_get_opmode() != STATIONAP_MODE) {
		return true;
	}

	ip_info info;
	wifi_get_ip_info(STATION_IF, &info);

	if(info.ip.addr == 0) {
		return false;
	}

	for(netif* netif = netif_list; netif != nullptr; netif = netif->next) {
		if(netif->ip_addr.addr == info.ip.addr) {
			debug_d("Fixed default network interface: %d.%d.%d.%d", IP2STR(&info.ip));
			netif_default = netif;
			ipClientRoutingFixed = true;
			return true;
		}
	}

	return false;
}
#endif

/////////////////////////////////

// enum tcp_state, see file /include/lwip/tcp.h
DEFINE_FSTR_LOCAL(deb_tcp_state_str, "CLOSED\0"		 // 0  CLOSED
									 "LISTEN\0"		 // 1  LISTEN
									 "SYN_SENT\0"	// 2  SYN_SENT
									 "SYN_RCVD\0"	// 3  SYN_RCVD
									 "ESTABLISHED\0" // 4  ESTABLISHED
									 "FIN_WAIT_1\0"  // 5  FIN_WAIT_1
									 "FIN_WAIT_2\0"  // 6  FIN_WAIT_2
									 "CLOSE_WAIT\0"  // 7  CLOSE_WAIT
									 "CLOSING\0"	 // 8  CLOSING
									 "LAST_ACK\0"	// 9  LAST_ACK
									 "TIME_WAIT");   // 10 TIME_WAIT

static void debugPrintTcp(const char* type, const struct tcp_pcb* pcb)
{
	debugf("********** Lwip %s PCB states:", type);
	CStringArray stateStr(deb_tcp_state_str);
	unsigned active = 0;
	while(pcb != nullptr) {
		debugf("LWIP_DEBUG: Port %u | %u flg:%02x tmr:%04x %s", pcb->local_port, pcb->remote_port, pcb->flags, pcb->tmr,
			   stateStr[pcb->state]);
		active++;
		pcb = pcb->next;
	}

	if(active == 0) {
		debugf("LWIP_DEBUG: none");
	}
}

void NetUtils::debugPrintTcpList()
{
	debugPrintTcp(_F("Active"), tcp_active_pcbs);
	debugPrintTcp(_F("Listen"), tcp_listen_pcbs.pcbs);
	debugPrintTcp(_F("TIME-WAIT"), tcp_tw_pcbs);
}
