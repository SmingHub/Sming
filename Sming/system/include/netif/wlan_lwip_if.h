/*
 *  Copyright (c) 2010-2011 	Espressif System  
 *	Add pvv
*/

#ifndef _WLAN_LWIP_IF_H_
#define _WLAN_LWIP_IF_H_

#define LWIP_IF0_PRIO   28
#define LWIP_IF1_PRIO   29

enum {
	SIG_LWIP_RX = 0, 
};

struct ieee80211_conn;

struct netif * eagle_lwip_if_alloc(struct ieee80211_conn *conn, uint8 *macaddr, struct ip_info *info) ICACHE_FLASH_ATTR;
struct netif * eagle_lwip_getif(int index);

#ifndef IOT_SIP_MODE
err_t ieee80211_output_pbuf(struct netif *ifp, struct pbuf* pb);
#else
err_t ieee80211_output_pbuf(struct ieee80211_conn *conn, esf_buf *eb);
#endif

extern uint8 * hostname;
extern bool default_hostname; //  = true;

#endif /*  _WLAN_LWIP_IF_H_ */
