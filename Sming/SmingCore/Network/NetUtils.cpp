/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "NetUtils.h"

#include "../Wiring/WString.h"
#include "lwip/tcp_impl.h"

bool NetUtils::ipClientRoutingFixed = false;

/// HELPERS ///

int NetUtils::pbufFindChar(pbuf *buf, char wtf, int startPos /* = 0*/)
{
	int ofs = 0;

	while (buf->len <= startPos)
	{
		ofs += buf->len;
		startPos -= buf->len;
		buf = buf->next;
		if (buf == NULL) return -1;
	}

	do
	{
		for (int i = startPos; i < buf->len; i++)
		{
			char* sbuf = (char*)buf->payload;
			if (sbuf[i] == wtf)
			{
				//debugf("%d %d", ofs, i);
				return ofs + i;
			}
		}
		ofs += buf->len;
		buf = buf->next;
		startPos = 0;
	} while (buf != NULL);

	return -1;
}

bool NetUtils::pbufIsStrEqual(pbuf *buf, const char* compared, int startPos)
{
	int cur = startPos;

	while (buf->len <= cur)
	{
		cur -= buf->len;
		buf = buf->next;
		if (buf == NULL) return false;
	}

	for (const char* cmp = compared; *cmp; cmp++)
	{
		if (buf == NULL) return false;

		char &target = ((char*)buf->payload)[cur];
		if (target != *cmp)
			return false;

		cur++;
		if (cur >= buf->len)
		{
			cur = 0;
			buf = buf->next;
		}
	}

	return true;
}

int NetUtils::pbufFindStr(pbuf *buf, const char* wtf, int startPos /* = 0*/)
{
	int cur = startPos;
	if (startPos < 0) startPos = 0;
	if (wtf == NULL || strlen(wtf) == -1) return -1;

	while (true)
	{
		cur = pbufFindChar(buf, wtf[0], cur);
		if (cur == -1) return -1;

		if (pbufIsStrEqual(buf, wtf, cur))
			return cur;
		cur++;
	}

	return -1;
}

char* NetUtils::pbufAllocateStrCopy(pbuf *buf, int startPos, int length)
{
	char* stringPtr = new char[length + 1];
	stringPtr[length] = '\0';
	pbuf_copy_partial(buf, stringPtr, length, startPos);
	return stringPtr;
}

String NetUtils::pbufStrCopy(pbuf *buf, int startPos, int length)
{
	char* stringPtr = new char[length + 1];
	stringPtr[length] = '\0';
	pbuf_copy_partial(buf, stringPtr, length, startPos);
	String res = String(stringPtr);
	delete[] stringPtr;
	return res;
}

int NetUtils::cbufFindChar(char* buf, char wtf, int length, int startPos /* = 0*/) {
	if(buf == NULL) return -1;
	if (startPos < 0) startPos = 0;
	for (int i = startPos; i < length; i++) {
		if(buf[i] == wtf) {
			return i;
		}
	}
	return -1;
}

bool NetUtils::cbufStrEqual(char* buf, const char* compared, int length, int startPos) {
	int cur = startPos;
	if (startPos < 0) startPos = 0;
	if(startPos >= length) return false;
	for (const char* cmp = compared; *cmp; cmp++)
	{
		if (buf[cur] != *cmp)
			return false;
		cur++;
		if (cur >= length) return false;
	}
	return true;
}

int NetUtils::cbufFindStr(char* buf, const char* wtf, int length, int startPos /* = 0*/)
{
	int cur = startPos;
	if (startPos < 0) startPos = 0;
	if (wtf == NULL || strlen(wtf) == -1) return -1;

	while (true)
	{
		cur = cbufFindChar(buf, wtf[0], length, cur);
		if (cur == -1) return -1;

		if (cbufStrEqual(buf, wtf, length, cur))
			return cur;
		cur++;
	}

	return -1;
}

String NetUtils::cbufStrCopy(char* buf, int startPos, int length) {
	char* stringPtr = new char[length + 1];
	stringPtr[length] = '\0';
	os_memcpy(stringPtr, &buf[startPos], length);
	String res = String(stringPtr);
	delete[] stringPtr;
	return res;
}

char* NetUtils::cbufAllocateStrCopy(char* buf, int startPos, int length)
{
	char* stringPtr = new char[length + 1];
	stringPtr[length] = '\0';
	os_memcpy(stringPtr, &buf[startPos], length);
	return stringPtr;
}

bool NetUtils::FixNetworkRouting()
{
//	if (ipClientRoutingFixed) return true;
//	if (wifi_get_opmode() != STATIONAP_MODE) return true;
//
//	ip_info info;
//	wifi_get_ip_info(STATION_IF, &info);
//
//	if (info.ip.addr == 0) return false;
//
//	for (netif *netif = netif_list; netif != NULL; netif = netif->next)
//	{
//		if (netif->ip_addr.addr == info.ip.addr)
//		{
//			debugf("Fixed default network interface: %d.%d.%d.%d", IP2STR(&info.ip));
//			netif_default = netif;
//			ipClientRoutingFixed = true;
//			return true;
//		}
//	}
//
//	return false;
	return true; // Should work on standard lwip
}

/////////////////////////////////

// enum tcp_state, see file /include/lwip/tcp.h
const char * const deb_tcp_state_str[] = {
  "CLOSED",      // 0  CLOSED
  "LISTEN",      // 1  LISTEN
  "SYN_SENT",    // 2  SYN_SENT
  "SYN_RCVD",    // 3  SYN_RCVD
  "ESTABLISHED", // 4  ESTABLISHED
  "FIN_WAIT_1",  // 5  FIN_WAIT_1
  "FIN_WAIT_2",  // 6  FIN_WAIT_2
  "CLOSE_WAIT",  // 7  CLOSE_WAIT
  "CLOSING",     // 8  CLOSING
  "LAST_ACK",    // 9  LAST_ACK
  "TIME_WAIT"    // 10 TIME_WAIT
};

void NetUtils::debugPrintTcpList()
{
	struct tcp_pcb *pcb;
	debugf("********** Lwip Active PCB states:\r\n");
	bool prt_none = true;
	for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
		debugf("LWIP_DEBUG: Port %u | %u flg:%02x tmr:%04x %s\r\n", pcb->local_port, pcb->remote_port, pcb->flags, pcb->tmr, deb_tcp_state_str[pcb->state]);
		prt_none = false;
	}
	if(prt_none) debugf("LWIP_DEBUG: none\r\n");
	debugf("********** Lwip Listen PCB states:\r\n");
	prt_none = true;
	for(pcb = (struct tcp_pcb *)tcp_listen_pcbs.pcbs; pcb != NULL; pcb = pcb->next) {
		debugf("LWIP_DEBUG: Port %u | %u flg:%02x tmr:%04x %s\r\n", pcb->local_port, pcb->remote_port, pcb->flags, pcb->tmr, deb_tcp_state_str[pcb->state]);
		prt_none = false;
	}
	if(prt_none) debugf("LWIP_DEBUG: none\r\n");
	debugf("********** Lwip TIME-WAIT PCB states:\r\n");
	prt_none = true;
	for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) {
		debugf("LWIP_DEBUG: Port %u | %u flg:%02x tmr:%04x %s\r\n", pcb->local_port, pcb->remote_port, pcb->flags, pcb->tmr, deb_tcp_state_str[pcb->state]);
		prt_none = false;
	}
	if(prt_none) debugf("LWIP_DEBUG: none\r\n");
	return;
}
