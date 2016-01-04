/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIP_NETIF_H__
#define __LWIP_NETIF_H__

#include "lwip/opt.h"

#define ENABLE_LOOPBACK (LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF)

#include "lwip/err.h"

#include "lwip/ip_addr.h"

#include "lwip/def.h"
#include "lwip/pbuf.h"
#if LWIP_DHCP
struct dhcp;
#endif
#if LWIP_AUTOIP
struct autoip;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Throughout this file, IP addresses are expected to be in
 * the same byte order as in IP_PCB. */

/** must be the maximum of all used hardware address lengths
    across all types of interfaces in use */
#define NETIF_MAX_HWADDR_LEN 6U

/** Whether the network interface is 'up'. This is
 * a software flag used to control whether this network
 * interface is enabled and processes traffic.
 * It is set by the startup code (for static IP configuration) or
 * by dhcp/autoip when an address has been assigned.
 */
#define NETIF_FLAG_UP           0x01U
/** If set, the netif has broadcast capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_BROADCAST    0x02U
/** If set, the netif is one end of a point-to-point connection.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_POINTTOPOINT 0x04U
/** If set, the interface is configured using DHCP.
 * Set by the DHCP code when starting or stopping DHCP. */
#define NETIF_FLAG_DHCP         0x08U
/** If set, the interface has an active link
 *  (set by the network interface driver).
 * Either set by the netif driver in its init function (if the link
 * is up at that time) or at a later point once the link comes up
 * (if link detection is supported by the hardware). */
#define NETIF_FLAG_LINK_UP      0x10U
/** If set, the netif is an ethernet device using ARP.
 * Set by the netif driver in its init function.
 * Used to check input packet types and use of DHCP. */
#define NETIF_FLAG_ETHARP       0x20U
/** If set, the netif is an ethernet device. It might not use
 * ARP or TCP/IP if it is used for PPPoE only.
 */
#define NETIF_FLAG_ETHERNET     0x40U
/** If set, the netif has IGMP capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_IGMP         0x80U

/** Function prototype for netif init functions. Set up flags and output/linkoutput
 * callback functions in this function.
 *
 * @param netif The netif to initialize
 */
typedef err_t (*netif_init_fn)(struct netif *netif);
/** Function prototype for netif->input functions. This function is saved as 'input'
 * callback function in the netif struct. Call it when a packet has been received.
 *
 * @param p The received packet, copied into a pbuf
 * @param inp The netif which received the packet
 */
typedef err_t (*netif_input_fn)(struct pbuf *p, struct netif *inp);
/** Function prototype for netif->output functions. Called by lwIP when a packet
 * shall be sent. For ethernet netif, set this to 'etharp_output' and set
 * 'linkoutput'.
 *
 * @param netif The netif which shall send a packet
 * @param p The packet to send (p->payload points to IP header)
 * @param ipaddr The IP address to which the packet shall be sent
 */
typedef err_t (*netif_output_fn)(struct netif *netif, struct pbuf *p,
       ip_addr_t *ipaddr);
/** Function prototype for netif->linkoutput functions. Only used for ethernet
 * netifs. This function is called by ARP when a packet shall be sent.
 *
 * @param netif The netif which shall send a packet
 * @param p The packet to send (raw ethernet packet)
 */
typedef err_t (*netif_linkoutput_fn)(struct netif *netif, struct pbuf *p);
/** Function prototype for netif status- or link-callback functions. */
typedef void (*netif_status_callback_fn)(struct netif *netif);
/** Function prototype for netif igmp_mac_filter functions */
typedef err_t (*netif_igmp_mac_filter_fn)(struct netif *netif,
       ip_addr_t *group, u8_t action);

/*add DHCP event processing by LiuHan*/
typedef void (*dhcp_event_fn)(void);

/** Generic data structure used for all lwIP network interfaces.
 *  The following fields should be filled in by the initialization
 *  function for the device driver: hwaddr_len, hwaddr[], mtu, flags */
struct netif {
  /** pointer to next in linked list */
  struct netif *next; //+00

  /** IP address configuration in network byte order */
  ip_addr_t ip_addr; // +04
  ip_addr_t netmask; // +08
  ip_addr_t gw; // +12

  /** This function is called by the network device driver
   *  to pass a packet up the TCP/IP stack. ��IP���������ݰ�*/
  netif_input_fn input; // +16
  /** This function is called by the IP module when it wants
   *  to send a packet on the interface. This function typically
   *  first resolves the hardware address, then sends the packet. ����IP���ݰ�*/
  netif_output_fn output; // +20
  /** This function is called by the ARP module when it wants
   *  to send a packet on the interface. This function outputs
   *  the pbuf as-is on the link medium. �ײ����ݰ�����*/
  netif_linkoutput_fn linkoutput; // +24
#if LWIP_NETIF_STATUS_CALLBACK
  /** This function is called when the netif state is set to up or down
   */
  netif_status_callback_fn status_callback;
#endif /* LWIP_NETIF_STATUS_CALLBACK */
#if LWIP_NETIF_LINK_CALLBACK
  /** This function is called when the netif link is set to up or down
   */
  netif_status_callback_fn link_callback;
#endif /* LWIP_NETIF_LINK_CALLBACK */
  /** This field can be set by the device driver and could point
   *  to state information for the device. ���������ֶΣ�����ָ��ײ��豸�����Ϣ*/
  void *state; // +28
#if LWIP_DHCP
  /** the DHCP client state information for this netif */
  struct dhcp *dhcp; // +32
  struct udp_pcb *dhcps_pcb; //+36	//dhcps
  dhcp_event_fn dhcp_event; // +40
#endif /* LWIP_DHCP */
#if LWIP_AUTOIP
  /** the AutoIP client state information for this netif */
  struct autoip *autoip;
#endif
#if LWIP_NETIF_HOSTNAME
  /* the hostname for this netif, NULL is a valid value */
  char*  hostname; // + 44
#endif /* LWIP_NETIF_HOSTNAME */
  /** maximum transfer unit (in bytes) �ýӿ������������ݰ����ȣ�����1500*/
  u16_t mtu; // + 48
  /** number of bytes used in hwaddr�ýӿ������ַ���� */
  u8_t hwaddr_len; // +50
  /** link level hardware address of this interface �ýӿ������ַ*/
  u8_t hwaddr[NETIF_MAX_HWADDR_LEN]; // +51 [6]
  /** flags (see NETIF_FLAG_ above) �ýӿ�״̬�������ֶ�*/
  u8_t flags; // +57
  /** descriptive abbreviation �ýӿڵ�����*/
  char name[2]; // +58
  /** number of this interface �ýӿڵı��*/
  u8_t num; // +60
#if LWIP_SNMP
  /** link type (from "snmp_ifType" enum from snmp.h) */
  u8_t link_type;
  /** (estimate) link speed */
  u32_t link_speed;
  /** timestamp at last change made (up/down) */
  u32_t ts;
  /** counters */
  u32_t ifinoctets;
  u32_t ifinucastpkts;
  u32_t ifinnucastpkts;
  u32_t ifindiscards;
  u32_t ifoutoctets;
  u32_t ifoutucastpkts;
  u32_t ifoutnucastpkts;
  u32_t ifoutdiscards;
#endif /* LWIP_SNMP */
#if LWIP_IGMP
  /** This function could be called to add or delete a entry in the multicast
      filter table of the ethernet MAC.*/
  netif_igmp_mac_filter_fn igmp_mac_filter; // +64
#endif /* LWIP_IGMP */
#if LWIP_NETIF_HWADDRHINT
  u8_t *addr_hint;
#endif /* LWIP_NETIF_HWADDRHINT */
#if ENABLE_LOOPBACK
  /* List of packets to be queued for ourselves. ָ���͸��Լ������ݰ���pbuf*/
  struct pbuf *loop_first;//��һ��
  struct pbuf *loop_last;//���һ��
#if LWIP_LOOPBACK_MAX_PBUFS
  u16_t loop_cnt_current;
#endif /* LWIP_LOOPBACK_MAX_PBUFS */
#endif /* ENABLE_LOOPBACK */
}; // [68]

#if LWIP_SNMP
#define NETIF_INIT_SNMP(netif, type, speed) \
  /* use "snmp_ifType" enum from snmp.h for "type", snmp_ifType_ethernet_csmacd by example */ \
  (netif)->link_type = (type);    \
  /* your link speed here (units: bits per second) */  \
  (netif)->link_speed = (speed);  \
  (netif)->ts = 0;              \
  (netif)->ifinoctets = 0;      \
  (netif)->ifinucastpkts = 0;   \
  (netif)->ifinnucastpkts = 0;  \
  (netif)->ifindiscards = 0;    \
  (netif)->ifoutoctets = 0;     \
  (netif)->ifoutucastpkts = 0;  \
  (netif)->ifoutnucastpkts = 0; \
  (netif)->ifoutdiscards = 0
#else /* LWIP_SNMP */
#define NETIF_INIT_SNMP(netif, type, speed)
#endif /* LWIP_SNMP */


/** The list of network interfaces. */
extern struct netif *netif_list;
/** The default network interface. */
extern struct netif *netif_default;

void netif_init(void)ICACHE_FLASH_ATTR;

struct netif *netif_add(struct netif *netif, ip_addr_t *ipaddr, ip_addr_t *netmask,
      ip_addr_t *gw, void *state, netif_init_fn init, netif_input_fn input)ICACHE_FLASH_ATTR;

void
netif_set_addr(struct netif *netif, ip_addr_t *ipaddr, ip_addr_t *netmask,
      ip_addr_t *gw)ICACHE_FLASH_ATTR;
void netif_remove(struct netif * netif)ICACHE_FLASH_ATTR;

/* Returns a network interface given its name. The name is of the form
   "et0", where the first two letters are the "name" field in the
   netif structure, and the digit is in the num field in the same
   structure. */
struct netif *netif_find(char *name)ICACHE_FLASH_ATTR;

void netif_set_default(struct netif *netif)ICACHE_FLASH_ATTR;

void netif_set_ipaddr(struct netif *netif, ip_addr_t *ipaddr)ICACHE_FLASH_ATTR;
void netif_set_netmask(struct netif *netif, ip_addr_t *netmask)ICACHE_FLASH_ATTR;
void netif_set_gw(struct netif *netif, ip_addr_t *gw)ICACHE_FLASH_ATTR;

void netif_set_up(struct netif *netif)ICACHE_FLASH_ATTR;
void netif_set_down(struct netif *netif)ICACHE_FLASH_ATTR;
/** Ask if an interface is up */
#define netif_is_up(netif) (((netif)->flags & NETIF_FLAG_UP) ? (u8_t)1 : (u8_t)0)

#if LWIP_NETIF_STATUS_CALLBACK
void netif_set_status_callback(struct netif *netif, netif_status_callback_fn status_callback)ICACHE_FLASH_ATTR;
#endif /* LWIP_NETIF_STATUS_CALLBACK */

void netif_set_link_up(struct netif *netif)ICACHE_FLASH_ATTR;
void netif_set_link_down(struct netif *netif)ICACHE_FLASH_ATTR;
/** Ask if a link is up */ 
#define netif_is_link_up(netif) (((netif)->flags & NETIF_FLAG_LINK_UP) ? (u8_t)1 : (u8_t)0)

#if LWIP_NETIF_LINK_CALLBACK
void netif_set_link_callback(struct netif *netif, netif_status_callback_fn link_callback)ICACHE_FLASH_ATTR;
#endif /* LWIP_NETIF_LINK_CALLBACK */

#if LWIP_NETIF_HOSTNAME
#define netif_set_hostname(netif, name) do { if((netif) != NULL) { (netif)->hostname = name; }}while(0)
#define netif_get_hostname(netif) (((netif) != NULL) ? ((netif)->hostname) : NULL)
#endif /* LWIP_NETIF_HOSTNAME */

#if LWIP_IGMP
#define netif_set_igmp_mac_filter(netif, function) do { if((netif) != NULL) { (netif)->igmp_mac_filter = function; }}while(0)
#define netif_get_igmp_mac_filter(netif) (((netif) != NULL) ? ((netif)->igmp_mac_filter) : NULL)
#endif /* LWIP_IGMP */

#if ENABLE_LOOPBACK
err_t netif_loop_output(struct netif *netif, struct pbuf *p, ip_addr_t *dest_ip)ICACHE_FLASH_ATTR;
void netif_poll(struct netif *netif)ICACHE_FLASH_ATTR;
#if !LWIP_NETIF_LOOPBACK_MULTITHREADING
void netif_poll_all(void)ICACHE_FLASH_ATTR;
#endif /* !LWIP_NETIF_LOOPBACK_MULTITHREADING */
#endif /* ENABLE_LOOPBACK */

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_NETIF_H__ */
