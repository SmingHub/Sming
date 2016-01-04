/*
 	Ð‘Ð°Ð·Ð¾Ð²Ñ‹Ð¹ ÐºÐ¾Ð´ Ð¾Ñ‚ Espressif
 	Ð˜Ð¼ÐµÐ½ÐµÐ½Ð¸Ñ� Ð¸ Ð´Ð¾Ð±Ð°Ð²Ð»ÐµÐ½Ð¸Ñ�: 	pvvx
*/
#include "user_config.h"


#include "lwip/inet.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/mem.h"
#include "osapi.h"

#ifdef USE_OPEN_DHCPS

#include "lwip/app/dhcpserver.h"

#ifndef LWIP_OPEN_SRC
#include "net80211/ieee80211_var.h"
#endif
#include "netif/wlan_lwip_if.h"
#include "user_interface.h"


#ifdef USE_CAPTDNS
#define USE_DNS
#endif

// https://tools.ietf.org/html/rfc2132

extern int wifi_softap_set_station_info(uint8_t * chaddr, struct ip_addr *ip);

#ifdef MEMLEAK_DEBUG
static const char mem_debug_file[] ICACHE_RODATA_ATTR = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////////////
//static const uint8_t xid[4] = {0xad, 0xde, 0x12, 0x23};
//static const uint8_t magic_cookie[4] = {99, 130, 83, 99};
//static u8_t old_xid[4] = {0};
static const uint32 magic_cookie ICACHE_RODATA_ATTR = 0x63538263;
static struct udp_pcb *pcb_dhcps LWIP_DATA_IRAM_ATTR; // = NULL;
static struct ip_addr broadcast_dhcps;
static struct ip_addr server_address;
static struct ip_addr client_address; //added
static struct ip_addr client_address_plus;

//static
struct dhcps_lease dhcps_lease;
uint32 dhcps_lease_flag LWIP_DATA_IRAM_ATTR; // = FALSE;
static list_node *plist LWIP_DATA_IRAM_ATTR; // = NULL;
static uint8 offer = 0xFF;
static uint32 renew LWIP_DATA_IRAM_ATTR; // = false;
#define DHCPS_LEASE_TIME_DEF	(120)
uint32 dhcps_lease_time LWIP_DATA_IRAM_ATTR; // = DHCPS_LEASE_TIME_DEF;  //minute -> init in dhcps_start()
/******************************************************************************
 * FunctionName : node_insert_to_list
 * Description  : insert the node to the list
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR node_insert_to_list(list_node **phead, list_node* pinsert)
{
	list_node *plist = NULL;
	struct dhcps_pool *pdhcps_pool = NULL;
	struct dhcps_pool *pdhcps_node = NULL;
	if (*phead == NULL)
		*phead = pinsert;
	else {
		plist = *phead;
		pdhcps_node = pinsert->pnode;
		pdhcps_pool = plist->pnode;

		if(pdhcps_node->ip.addr < pdhcps_pool->ip.addr) {
		    pinsert->pnext = plist;
		    *phead = pinsert;
		} else {
            while (plist->pnext != NULL) {
                pdhcps_pool = plist->pnext->pnode;
                if (pdhcps_node->ip.addr < pdhcps_pool->ip.addr) {
                    pinsert->pnext = plist->pnext;
                    plist->pnext = pinsert;
                    break;
                }
                plist = plist->pnext;
            }

            if(plist->pnext == NULL) {
                plist->pnext = pinsert;
            }
		}
	}
//	pinsert->pnext = NULL;
}

/******************************************************************************
 * FunctionName : node_delete_from_list
 * Description  : remove the node from list
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR node_remove_from_list(list_node **phead, list_node* pdelete)
{
	list_node *plist = NULL;

	plist = *phead;
	if (plist == NULL){
		*phead = NULL;
	} else {
		if (plist == pdelete){
			*phead = plist->pnext;
		} else {
			while (plist != NULL) {
				if (plist->pnext == pdelete){
					plist->pnext = pdelete->pnext;
				}
				plist = plist->pnext;
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ï¿½ï¿½DHCP msgï¿½ï¿½Ï¢ï¿½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
 *
 * @param optptr -- DHCP msgï¿½ï¿½Ï¢Î»ï¿½ï¿½
 * @param type -- Òªï¿½ï¿½Óµï¿½ï¿½ï¿½ï¿½ï¿½option
 *
 * @return uint8_t* ï¿½ï¿½ï¿½ï¿½DHCP msgÆ«ï¿½Æµï¿½Ö·
 */
///////////////////////////////////////////////////////////////////////////////////
static uint8_t* ICACHE_FLASH_ATTR add_msg_type(uint8_t *optptr, uint8_t type)
{

        *optptr++ = DHCP_OPTION_MSG_TYPE;
        *optptr++ = 1;
        *optptr++ = type;
        return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ï¿½ï¿½DHCP msgï¿½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½offerÓ¦ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
 *
 * @param optptr -- DHCP msgï¿½ï¿½Ï¢Î»ï¿½ï¿½
 *
 * @return uint8_t* ï¿½ï¿½ï¿½ï¿½DHCP msgÆ«ï¿½Æµï¿½Ö·
 */
///////////////////////////////////////////////////////////////////////////////////
static uint8_t* ICACHE_FLASH_ATTR add_offer_options(uint8_t *optptr)
{
        struct ip_addr ipadd;

        ipadd.addr = *( (uint32_t *) &server_address);

#ifdef USE_CLASS_B_NET
        *optptr++ = DHCP_OPTION_SUBNET_MASK;
        *optptr++ = 4;  //length
        *optptr++ = 255;
        *optptr++ = 240;	
        *optptr++ = 0;
        *optptr++ = 0;
#else
        *optptr++ = DHCP_OPTION_SUBNET_MASK;
        *optptr++ = 4;  
        *optptr++ = 255;
        *optptr++ = 255;	
        *optptr++ = 255;
        *optptr++ = 0;
#endif

        *optptr++ = DHCP_OPTION_LEASE_TIME;
        *optptr++ = 4;  
        *optptr++ = ((dhcps_lease_time * 60) >> 24);// & 0xFF;
        *optptr++ = ((dhcps_lease_time * 60) >> 16);// & 0xFF;
        *optptr++ = ((dhcps_lease_time * 60) >> 8);// & 0xFF;
        *optptr++ = ((dhcps_lease_time * 60) >> 0);// & 0xFF;

        *optptr++ = DHCP_OPTION_SERVER_ID;
        *optptr++ = 4;  
        *optptr++ = ip4_addr1( &ipadd);
        *optptr++ = ip4_addr2( &ipadd);
        *optptr++ = ip4_addr3( &ipadd);
        *optptr++ = ip4_addr4( &ipadd);

        if (dhcps_router_enabled(offer)){
        	struct ip_info if_ip;
			os_bzero(&if_ip, sizeof(struct ip_info));
			wifi_get_ip_info(SOFTAP_IF, &if_ip);

			*optptr++ = DHCP_OPTION_ROUTER;
			*optptr++ = 4;
			*optptr++ = ip4_addr1( &if_ip.gw);
			*optptr++ = ip4_addr2( &if_ip.gw);
			*optptr++ = ip4_addr3( &if_ip.gw);
			*optptr++ = ip4_addr4( &if_ip.gw);
        }

#ifdef USE_DNS
	    *optptr++ = DHCP_OPTION_DNS_SERVER;
	    *optptr++ = 4;
	    *optptr++ = ip4_addr1( &ipadd);
		*optptr++ = ip4_addr2( &ipadd);
		*optptr++ = ip4_addr3( &ipadd);
		*optptr++ = ip4_addr4( &ipadd);
#endif

#ifdef CLASS_B_NET
        *optptr++ = DHCP_OPTION_BROADCAST_ADDRESS;
        *optptr++ = 4;  
        *optptr++ = ip4_addr1( &ipadd);
        *optptr++ = 255;
        *optptr++ = 255;
        *optptr++ = 255;
#else
        *optptr++ = DHCP_OPTION_BROADCAST_ADDRESS;
        *optptr++ = 4;  
        *optptr++ = ip4_addr1( &ipadd);
        *optptr++ = ip4_addr2( &ipadd);
        *optptr++ = ip4_addr3( &ipadd);
        *optptr++ = 255;
#endif

        *optptr++ = DHCP_OPTION_INTERFACE_MTU;
        *optptr++ = 2;  
#ifdef CLASS_B_NET
        *optptr++ = 0x05;	
        *optptr++ = 0xdc;
#else
        *optptr++ = 0x02;	
        *optptr++ = 0x40;
#endif

        *optptr++ = DHCP_OPTION_PERFORM_ROUTER_DISCOVERY;
        *optptr++ = 1;  
        *optptr++ = 0x00; 

        *optptr++ = 43;	
        *optptr++ = 6;	

        *optptr++ = 0x01;	
        *optptr++ = 4;  
        *optptr++ = 0x00;
        *optptr++ = 0x00;
        *optptr++ = 0x00;
        *optptr++ = 0x02; 	

#if 0 //def USE_NETBIOS over TCP/IP
        if(syscfg.cfg.b.netbios_ena) {
            *optptr++ = 44; // NetBIOS over TCP/IP Name Server Option
            *optptr++ = 4;
    		*optptr++ = ip4_addr1( &ipadd);
            *optptr++ = ip4_addr2( &ipadd);
            *optptr++ = ip4_addr3( &ipadd);
            *optptr++ = ip4_addr4( &ipadd);
/*
            *optptr++ = 45; // NetBIOS over TCP/IP Datagram Distribution Server Option
            *optptr++ = 4;
    		*optptr++ = ip4_addr1( &ipadd);
            *optptr++ = ip4_addr2( &ipadd);
            *optptr++ = ip4_addr3( &ipadd);
            *optptr++ = ip4_addr4( &ipadd);
*/
            *optptr++ = 46; // NetBIOS over TCP/IP Node Type Option
            *optptr++ = 1;
            *optptr++ = 1; // B-node
        }
#endif
        return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ï¿½ï¿½DHCP msgï¿½á¹¹ï¿½ï¿½ï¿½ï¿½Ó½ï¿½ï¿½ï¿½ï¿½Ö¾ï¿½ï¿½ï¿½ï¿½
 *
 * @param optptr -- DHCP msgï¿½ï¿½Ï¢Î»ï¿½ï¿½
 *
 * @return uint8_t* ï¿½ï¿½ï¿½ï¿½DHCP msgÆ«ï¿½Æµï¿½Ö·
 */
///////////////////////////////////////////////////////////////////////////////////
static uint8_t* ICACHE_FLASH_ATTR add_end(uint8_t *optptr)
{

        *optptr++ = DHCP_OPTION_END;
        return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static void ICACHE_FLASH_ATTR create_msg(struct dhcps_msg *m)
{
        struct ip_addr client;

        client.addr = *( (uint32_t *) &client_address);

        m->op = DHCP_REPLY;
        m->htype = DHCP_HTYPE_ETHERNET;
        m->hlen = 6;  
        m->hops = 0;
//        os_memcpy((char *) xid, (char *) m->xid, sizeof(m->xid));
        m->secs = 0;
        m->flags = htons(BOOTP_BROADCAST); 

        os_memcpy((char *) m->yiaddr, (char *) &client.addr, sizeof(m->yiaddr));

        os_memset((char *) m->ciaddr, 0, sizeof(m->ciaddr));
        os_memset((char *) m->siaddr, 0, sizeof(m->siaddr));
        os_memset((char *) m->giaddr, 0, sizeof(m->giaddr));
        os_memset((char *) m->sname, 0, sizeof(m->sname));
        os_memset((char *) m->file, 0, sizeof(m->file));

        os_memset((char *) m->options, 0, sizeof(m->options));
        os_memcpy((char *) m->options, &magic_cookie, sizeof(magic_cookie));
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½OFFER
 *
 * @param -- m Ö¸ï¿½ï¿½ï¿½ï¿½Òªï¿½ï¿½ï¿½Íµï¿½DHCP msgï¿½ï¿½ï¿½ï¿½
 */
///////////////////////////////////////////////////////////////////////////////////
static void ICACHE_FLASH_ATTR send_offer(struct dhcps_msg *m)
{
        uint8_t *end;
	    struct pbuf *p, *q;
	    u8_t *data;
	    u16_t cnt=0;
	    u16_t i;
        create_msg(m);

        end = add_msg_type(&m->options[4], DHCPOFFER);
        end = add_offer_options(end);
        end = add_end(end);

	    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct dhcps_msg), PBUF_RAM);
#if DHCPS_DEBUG
		os_printf("udhcp: send_offer>>p->ref = %d\n", p->ref);
#endif
	    if(p != NULL){
	       
#if DHCPS_DEBUG
	        os_printf("dhcps: send_offer>>pbuf_alloc succeed\n");
	        os_printf("dhcps: send_offer>>p->tot_len = %d\n", p->tot_len);
	        os_printf("dhcps: send_offer>>p->len = %d\n", p->len);
#endif
	        q = p;
	        while(q != NULL){
	            data = (u8_t *)q->payload;
	            for(i=0; i<q->len; i++)
	            {
	                data[i] = ((u8_t *) m)[cnt++];
#if DHCPS_DEBUG
					os_printf("%02x ",data[i]);
					if((i+1)%16 == 0){
						os_printf("\n");
					}
#endif
	            }

	            q = q->next;
	        }
	    }else{
	        
#if DHCPS_DEBUG
	        os_printf("dhcps: send_offer>>pbuf_alloc failed\n");
#endif
	        return;
	    }
#if DHCPS_DEBUG
	    err_t SendOffer_err_t = udp_sendto( pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT );
	    os_printf("dhcps: send_offer>>udp_sendto result %x\n", SendOffer_err_t);
#else
	    udp_sendto( pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT );
#endif
	    if(p->ref != 0){	
#if DHCPS_DEBUG
	        os_printf("udhcp: send_offer>>free pbuf\n");
#endif
	        pbuf_free(p);
	    }
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½NAKï¿½ï¿½Ï¢
 *
 * @param m Ö¸ï¿½ï¿½ï¿½ï¿½Òªï¿½ï¿½ï¿½Íµï¿½DHCP msgï¿½ï¿½ï¿½ï¿½
 */
///////////////////////////////////////////////////////////////////////////////////
static void ICACHE_FLASH_ATTR send_nak(struct dhcps_msg *m)
{

    	u8_t *end;
	    struct pbuf *p, *q;
	    u8_t *data;
	    u16_t cnt=0;
	    u16_t i;
        create_msg(m);

        end = add_msg_type(&m->options[4], DHCPNAK);
        end = add_end(end);

	    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct dhcps_msg), PBUF_RAM);
#if DHCPS_DEBUG
		os_printf("udhcp: send_nak>>p->ref = %d\n", p->ref);
#endif
	    if(p != NULL){
	        
#if DHCPS_DEBUG
	        os_printf("dhcps: send_nak>>pbuf_alloc succeed\n");
	        os_printf("dhcps: send_nak>>p->tot_len = %d\n", p->tot_len);
	        os_printf("dhcps: send_nak>>p->len = %d\n", p->len);
#endif
	        q = p;
	        while(q != NULL){
	            data = (u8_t *)q->payload;
	            for(i=0; i<q->len; i++)
	            {
	                data[i] = ((u8_t *) m)[cnt++];
#if DHCPS_DEBUG					
					os_printf("%02x ",data[i]);
					if((i+1)%16 == 0){
						os_printf("\n");
					}
#endif
	            }

	            q = q->next;
	        }
	    }else{
	        
#if DHCPS_DEBUG
	        os_printf("dhcps: send_nak>>pbuf_alloc failed\n");
#endif
	        return;
    	}
#if DHCPS_DEBUG
		err_t SendNak_err_t = udp_sendto( pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT );
        os_printf("dhcps: send_nak>>udp_sendto result %x\n",SendNak_err_t);
#else
        udp_sendto( pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT );
#endif
 	    if(p->ref != 0){
#if DHCPS_DEBUG			
	        os_printf("udhcp: send_nak>>free pbuf\n");
#endif
	        pbuf_free(p);
	    }
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ACKï¿½ï¿½DHCPï¿½Í»ï¿½ï¿½ï¿½
 *
 * @param m Ö¸ï¿½ï¿½ï¿½ï¿½Òªï¿½ï¿½ï¿½Íµï¿½DHCP msgï¿½ï¿½ï¿½ï¿½
 */
///////////////////////////////////////////////////////////////////////////////////
static void ICACHE_FLASH_ATTR send_ack(struct dhcps_msg *m)
{

		u8_t *end;
	    struct pbuf *p, *q;
	    u8_t *data;
	    u16_t cnt=0;
	    u16_t i;
        create_msg(m);

        end = add_msg_type(&m->options[4], DHCPACK);
        end = add_offer_options(end);
        end = add_end(end);
	    
	    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct dhcps_msg), PBUF_RAM);
#if DHCPS_DEBUG
		os_printf("udhcp: send_ack>>p->ref = %d\n", p->ref);
#endif
	    if(p != NULL){
	        
#if DHCPS_DEBUG
	        os_printf("dhcps: send_ack>>pbuf_alloc succeed\n");
	        os_printf("dhcps: send_ack>>p->tot_len = %d\n", p->tot_len);
	        os_printf("dhcps: send_ack>>p->len = %d\n", p->len);
#endif
	        q = p;
	        while(q != NULL){
	            data = (u8_t *)q->payload;
	            for(i=0; i<q->len; i++)
	            {
	                data[i] = ((u8_t *) m)[cnt++];
#if DHCPS_DEBUG					
					os_printf("%02x ",data[i]);
					if((i+1)%16 == 0){
						os_printf("\n");
					}
#endif
	            }

	            q = q->next;
	        }
	    }else{
	    
#if DHCPS_DEBUG
	        os_printf("dhcps: send_ack>>pbuf_alloc failed\n");
#endif
	        return;
	    }
#if DHCPS_DEBUG
		err_t SendAck_err_t = udp_sendto( pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT );
		os_printf("dhcps: send_ack>>udp_sendto result %x\n",SendAck_err_t);
#else
		udp_sendto( pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT );
#endif
	    if(p->ref != 0){
#if DHCPS_DEBUG
	        os_printf("udhcp: send_ack>>free pbuf\n");
#endif
	        pbuf_free(p);
	    }
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ï¿½ï¿½ï¿½ï¿½DHCPï¿½Í»ï¿½ï¿½Ë·ï¿½ï¿½ï¿½ï¿½ï¿½DHCPï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï¢ï¿½ï¿½ï¿½ï¿½ï¿½Ô²ï¿½Í¬ï¿½ï¿½DHCPï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ó¦ï¿½ï¿½Ó¦ï¿½ï¿½
 *
 * @param optptr DHCP msgï¿½Ðµï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
 * @param len ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä´ï¿½ï¿½?(byte)
 *
 * @return uint8_t ï¿½ï¿½ï¿½Ø´ï¿½ï¿½ï¿½ï¿½ï¿½DHCP Server×´Ì¬Öµ
 */
///////////////////////////////////////////////////////////////////////////////////
static uint8_t ICACHE_FLASH_ATTR parse_options(uint8_t *optptr, sint16_t len)
{
        struct ip_addr client;
    	bool is_dhcp_parse_end = false;
    	struct dhcps_state s;

        client.addr = *( (uint32_t *) &client_address);// Òªï¿½ï¿½ï¿½ï¿½ï¿½DHCPï¿½Í»ï¿½ï¿½Ëµï¿½IP

        u8_t *end = optptr + len;
        u16_t type = 0;

        s.state = DHCPS_STATE_IDLE;

        while (optptr < end) {
#if DHCPS_DEBUG
        	os_printf("dhcps: (sint16_t)*optptr = %d\n", (sint16_t)*optptr);
#endif
        	switch ((sint16_t) *optptr) {

                case DHCP_OPTION_MSG_TYPE:	//53
                        type = *(optptr + 2);
                        break;

                case DHCP_OPTION_REQ_IPADDR: //50
                        if( os_memcmp( (char *) &client.addr, (char *) optptr+2,4)==0 ) {
#if DHCPS_DEBUG
                    		os_printf("dhcps: DHCP_OPTION_REQ_IPADDR = 0 ok\n");
#endif
                            s.state = DHCPS_STATE_ACK;
                        }else {
#if DHCPS_DEBUG
                    		os_printf("dhcps: DHCP_OPTION_REQ_IPADDR != 0 err\n");
#endif
                            s.state = DHCPS_STATE_NAK;
                        }
                        break;
                case DHCP_OPTION_END:
			            {
			                is_dhcp_parse_end = true;
			            }
                        break;
            }

		    if(is_dhcp_parse_end){
		            break;
		    }

            optptr += optptr[1] + 2;
        }

        switch (type){
        
        	case DHCPDISCOVER: //1
                s.state = DHCPS_STATE_OFFER;
#if DHCPS_DEBUG
            	os_printf("dhcps: DHCPD_STATE_OFFER\n");
#endif
                break;

        	case DHCPREQUEST: //3
                if ( !(s.state == DHCPS_STATE_ACK || s.state == DHCPS_STATE_NAK) ) {
                    if(renew == true) {
                        s.state = DHCPS_STATE_ACK;
                    } else {
                        s.state = DHCPS_STATE_NAK;
                    }
#if DHCPS_DEBUG
                		os_printf("dhcps: DHCPD_STATE_NAK\n");
#endif
                }
                break;

			case DHCPDECLINE: //4
                s.state = DHCPS_STATE_IDLE;
#if DHCPS_DEBUG
            	os_printf("dhcps: DHCPD_STATE_IDLE\n");
#endif
                break;

        	case DHCPRELEASE: //7
                s.state = DHCPS_STATE_RELEASE;
#if DHCPS_DEBUG
            	os_printf("dhcps: DHCPD_STATE_IDLE\n");
#endif
                break;
        }
#if DHCPS_DEBUG
    	os_printf("dhcps: return s.state = %d\n", s.state);
#endif
        return s.state;
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static sint16_t ICACHE_FLASH_ATTR parse_msg(struct dhcps_msg *m, u16_t len)
{
/*	if(os_memcmp((char *)m->options,
          &magic_cookie,
          sizeof(magic_cookie)) == 0){ */
		if((m->options[0] == 0x63) && (m->options[1] == 0x82) && (m->options[2] == 0x53) && (m->options[3]==0x63)) {
#if DHCPS_DEBUG
        	os_printf("dhcps: len = %d\n", len);
#endif
	        /*
         	 * ï¿½ï¿½Â¼ï¿½ï¿½Ç°ï¿½ï¿½xidï¿½ï¿½ï¿½ï´¦ï¿½ï¿½ï¿½?
         	 * ï¿½Ëºï¿½ÎªDHCPï¿½Í»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ã»ï¿½Í³Ò»ï¿½ï¿½È¡IPÊ±ï¿½ï¿½
         	*/
//	        if((old_xid[0] == 0) &&
//	           (old_xid[1] == 0) &&
//	           (old_xid[2] == 0) &&
//	           (old_xid[3] == 0)){
//	            /*
//	             * old_xidÎ´ï¿½ï¿½Â¼ï¿½Îºï¿½ï¿½ï¿½ï¿½?
//	             * ï¿½Ï¶ï¿½ï¿½Çµï¿½Ò»ï¿½ï¿½Ê¹ï¿½ï¿½
//	            */
//	            os_memcpy((char *)old_xid, (char *)m->xid, sizeof(m->xid));
//	        }else{
//	            /*
//	             * ï¿½ï¿½ï¿½Î´ï¿½ï¿½ï¿½ï¿½DHCP msgï¿½ï¿½Ð¯ï¿½ï¿½ï¿½xidï¿½ï¿½ï¿½Ï´Î¼ï¿½Â¼ï¿½Ä²ï¿½Í¬ï¿½ï¿½
//	             * ï¿½Ï¶ï¿½Îªï¿½ï¿½Í¬ï¿½ï¿½DHCPï¿½Í»ï¿½ï¿½Ë·ï¿½ï¿½Í£ï¿½ï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½Òªï¿½ï¿½ï¿½ï¿½Ä¿Í»ï¿½ï¿½ï¿½IP
//	             * ï¿½ï¿½ï¿½ï¿½ 192.168.4.100(0x6404A8C0) <--> 192.168.4.200(0xC804A8C0)
//	             *
//	            */
//	            if(os_memcmp((char *)old_xid, (char *)m->xid, sizeof(m->xid)) != 0){
	                /*
                 	 * ï¿½ï¿½Â¼ï¿½ï¿½ï¿½Îµï¿½xidï¿½Å£ï¿½Í¬Ê±ï¿½ï¿½ï¿½ï¿½ï¿½IPï¿½ï¿½ï¿½ï¿½
                 	*/
//	                struct ip_addr addr_tmp;
//	                os_memcpy((char *)old_xid, (char *)m->xid, sizeof(m->xid));

//	                {
						struct dhcps_pool *pdhcps_pool = NULL;
						list_node *pnode = NULL;
						list_node *pback_node = NULL;
						struct ip_addr first_address;
						bool flag = false;

//						POOL_START:
						first_address.addr = dhcps_lease.start_ip.addr;
						client_address.addr = client_address_plus.addr;
						renew = false;
//							addr_tmp.addr =  htonl(client_address_plus.addr);
//							addr_tmp.addr++;
//							client_address_plus.addr = htonl(addr_tmp.addr);
						for (pback_node = plist; pback_node != NULL;pback_node = pback_node->pnext) {
							pdhcps_pool = pback_node->pnode;
							if (os_memcmp(pdhcps_pool->mac, m->chaddr, sizeof(pdhcps_pool->mac)) == 0){
//									os_printf("the same device request ip\n");
								if (os_memcmp(&pdhcps_pool->ip.addr, m->ciaddr, sizeof(pdhcps_pool->ip.addr)) == 0) {
								    renew = true;
								}
								client_address.addr = pdhcps_pool->ip.addr;
								pdhcps_pool->lease_timer = dhcps_lease_time;
								pnode = pback_node;
								goto POOL_CHECK;
							} else if (pdhcps_pool->ip.addr == client_address_plus.addr){
//									client_address.addr = client_address_plus.addr;
//									os_printf("the ip addr has been request\n");
/*								addr_tmp.addr = htonl(client_address_plus.addr);
								addr_tmp.addr++;
								client_address_plus.addr = htonl(addr_tmp.addr);
								client_address.addr = client_address_plus.addr; */
								client_address.addr = client_address_plus.addr + (1<<24);
							}

							if(flag == false) { // search the fisrt unused ip
                                if(first_address.addr < pdhcps_pool->ip.addr) {
                                    flag = true;
                                } else {
/*                                    addr_tmp.addr = htonl(first_address.addr);
                                    addr_tmp.addr++;
                                    first_address.addr = htonl(addr_tmp.addr); */
                                    first_address.addr += (1<<24);
                                }
							}
						}
						if (client_address_plus.addr > dhcps_lease.end_ip.addr) {
						    client_address.addr = first_address.addr;
						}
						if (client_address.addr > dhcps_lease.end_ip.addr) {
						    client_address_plus.addr = dhcps_lease.start_ip.addr;
						    pdhcps_pool = NULL;
						    pnode = NULL;
						} else {
						    pdhcps_pool = (struct dhcps_pool *)os_zalloc(sizeof(struct dhcps_pool));
						    pdhcps_pool->ip.addr = client_address.addr;
						    os_memcpy(pdhcps_pool->mac, m->chaddr, sizeof(pdhcps_pool->mac));
						    pdhcps_pool->lease_timer = dhcps_lease_time;
						    pnode = (list_node *)os_zalloc(sizeof(list_node ));
						    pnode->pnode = pdhcps_pool;
						    pnode->pnext = NULL;
						    node_insert_to_list(&plist,pnode);
						    if (client_address.addr == dhcps_lease.end_ip.addr) {
						        client_address_plus.addr = dhcps_lease.start_ip.addr;
						    } else {
/*                                addr_tmp.addr = htonl(client_address.addr);
                                addr_tmp.addr++;
                                client_address_plus.addr = htonl(addr_tmp.addr); */
                                client_address_plus.addr = client_address.addr + (1<<24);
						    }
						}

						POOL_CHECK:
						if ((client_address.addr > dhcps_lease.end_ip.addr) || (client_address.addr == IPADDR_ANY)){
                            os_printf("client_address_plus.addr %x %d\n", client_address_plus.addr, system_get_free_heap_size());
						    if(pnode != NULL) {
						        node_remove_from_list(&plist,pnode);
						        os_free(pnode);
						        pnode = NULL;
						    }

						    if (pdhcps_pool != NULL) {
						        os_free(pdhcps_pool);
						        pdhcps_pool = NULL;
						    }
//							client_address_plus.addr = dhcps_lease.start_ip.addr;
							return 4;
						}

						sint16_t ret = parse_options(&m->options[4], len);;

						if(ret == DHCPS_STATE_RELEASE) {
						    if(pnode != NULL) {
						        node_remove_from_list(&plist,pnode);
						        os_free(pnode);
						        pnode = NULL;
						    }

						    if (pdhcps_pool != NULL) {
						        os_free(pdhcps_pool);
						        pdhcps_pool = NULL;
						    }
						    os_memset(&client_address,0x0,sizeof(client_address));
						}

						if (wifi_softap_set_station_info(m->chaddr, &client_address) == false) {
						    return 0;
						}
//	                }

#if DHCPS_DEBUG
	                os_printf("dhcps: xid changed\n");
	                os_printf("dhcps: client_address.addr = %x\n", client_address.addr);
#endif
	               
//	            }
	            
//	        }
	        return ret;
	    }
        return 0;
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * DHCP ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ý°ï¿½ï¿½ï¿½Õ´ï¿½ï¿½ï¿½Øµï¿½ï¿½ï¿½ï¿½ï¿½Ëºï¿½ï¿½ï¿½ï¿½ï¿½LWIP UDPÄ£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
 * ï¿½ï¿½Òªï¿½ï¿½ï¿½ï¿½udp_recv()ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½LWIPï¿½ï¿½ï¿½ï¿½×¢ï¿½ï¿½.
 *
 * @param arg
 * @param pcb ï¿½ï¿½ï¿½Õµï¿½UDPï¿½ï¿½Ä¿ï¿½ï¿½Æ¿ï¿½?
 * @param p ï¿½ï¿½ï¿½Õµï¿½ï¿½ï¿½UDPï¿½Ðµï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½?
 * @param addr ï¿½ï¿½ï¿½Í´ï¿½UDPï¿½ï¿½ï¿½Ô´ï¿½ï¿½ï¿½ï¿½ï¿½IPï¿½ï¿½Ö·
 * @param port ï¿½ï¿½ï¿½Í´ï¿½UDPï¿½ï¿½ï¿½Ô´ï¿½ï¿½ï¿½ï¿½ï¿½UDPÍ¨ï¿½ï¿½ï¿½Ë¿Úºï¿½
 */
///////////////////////////////////////////////////////////////////////////////////
static void ICACHE_FLASH_ATTR handle_dhcp(void *arg, 
									struct udp_pcb *pcb, 
									struct pbuf *p, 
									struct ip_addr *addr, 
									uint16_t port)
{
		struct dhcps_msg *pmsg_dhcps = NULL;
		sint16_t tlen = 0;
        u16_t i = 0;
	    u16_t dhcps_msg_cnt = 0;
	    u8_t *p_dhcps_msg = NULL;
	    u8_t *data = NULL;

#if DHCPS_DEBUG
    	os_printf("dhcps: handle_dhcp-> receive a packet\n");
#endif
	    if (p==NULL) return;

	    pmsg_dhcps = (struct dhcps_msg *)os_zalloc(sizeof(struct dhcps_msg));
	    if (NULL == pmsg_dhcps){
	    	pbuf_free(p);
	    	return;
	    }
	    p_dhcps_msg = (u8_t *)pmsg_dhcps;
		tlen = p->tot_len;
	    data = p->payload;

#if DHCPS_DEBUG
	    os_printf("dhcps: handle_dhcp-> p->tot_len = %d\n", tlen);
	    os_printf("dhcps: handle_dhcp-> p->len = %d\n", p->len);
#endif		

	    for(i=0; i<p->len; i++){
	        p_dhcps_msg[dhcps_msg_cnt++] = data[i];
#if DHCPS_DEBUG					
			os_printf("%02x ",data[i]);
			if((i+1)%16 == 0){
				os_printf("\n");
			}
#endif
	    }
		
		if(p->next != NULL) {
#if DHCPS_DEBUG
	        os_printf("dhcps: handle_dhcp-> p->next != NULL\n");
	        os_printf("dhcps: handle_dhcp-> p->next->tot_len = %d\n",p->next->tot_len);
	        os_printf("dhcps: handle_dhcp-> p->next->len = %d\n",p->next->len);
#endif
			
	        data = p->next->payload;
	        for(i=0; i<p->next->len; i++){
	            p_dhcps_msg[dhcps_msg_cnt++] = data[i];
#if DHCPS_DEBUG					
				os_printf("%02x ",data[i]);
				if((i+1)%16 == 0){
					os_printf("\n");
				}
#endif
			}
		}

		/*
	     * DHCP ï¿½Í»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï¢ï¿½ï¿½ï¿½ï¿½
	    */
#if DHCPS_DEBUG
    	os_printf("dhcps: handle_dhcp-> parse_msg(p)\n");
#endif
		
        switch(parse_msg(pmsg_dhcps, tlen - 240)) {

	        case DHCPS_STATE_OFFER: //1
#if DHCPS_DEBUG            
            	 os_printf("dhcps: handle_dhcp-> DHCPD_STATE_OFFER\n");
#endif			
	             send_offer(pmsg_dhcps);
	             break;
	        case DHCPS_STATE_ACK: //3
#if DHCPS_DEBUG
            	 os_printf("dhcps: handle_dhcp-> DHCPD_STATE_ACK\n");
#endif			
	             send_ack(pmsg_dhcps);
	             break;
	        case DHCPS_STATE_NAK: //4
#if DHCPS_DEBUG            
            	 os_printf("dhcps: handle_dhcp-> DHCPD_STATE_NAK\n");
#endif
	             send_nak(pmsg_dhcps);
	             break;
			default :
				 break;
        }
#if DHCPS_DEBUG
    	os_printf("dhcps: handle_dhcp-> pbuf_free(p)\n");
#endif
        pbuf_free(p);
        os_free(pmsg_dhcps);
        pmsg_dhcps = NULL;
}
///////////////////////////////////////////////////////////////////////////////////
static void ICACHE_FLASH_ATTR wifi_softap_init_dhcps_lease(uint32 ip)
{
	uint32 softap_ip = ip & ((1 << 24) - 1); // 0x00FFFFFF;
	if (dhcps_lease_flag == TRUE) { // Ð¿Ñ€Ð¾Ð²ÐµÑ€ÐºÐ°
		if(softap_ip != (dhcps_lease.start_ip.addr & ((1 << 24) - 1))
		 ||softap_ip != (dhcps_lease.end_ip.addr & ((1 << 24) - 1))
		 ||dhcps_lease.end_ip.addr - dhcps_lease.start_ip.addr > (DHCPS_MAX_LEASE << 24)
		 ||(ip >= dhcps_lease.start_ip.addr && ip <= dhcps_lease.end_ip.addr)) // Ð¿Ð¾Ð¿Ð°Ð´Ð°ÐµÑ‚ Ð² Ð´Ð¸Ð°Ð¿Ð°Ð·Ð¾Ð½
			dhcps_lease_flag = FALSE;
	}
	if (dhcps_lease_flag == FALSE) { // dhcps_lease Ð½Ðµ Ð·Ð°Ð´Ð°Ð²Ð°Ð»Ð°Ñ�ÑŒ -> Ð°Ð²Ñ‚Ð¾Ð¼Ð°Ñ‚Ð¸Ñ‡ÐµÑ�ÐºÐ°Ñ� ÑƒÑ�Ñ‚Ð°Ð½Ð¾Ð²ÐºÐ°
	    uint32 local_ip = ip & (255 << 24); // 0xFF000000;
		if (local_ip >= (128 << 24))
			local_ip -= (DHCPS_MAX_LEASE + 1) << 24;
		else
			local_ip += 1 << 24;
		dhcps_lease.start_ip.addr = softap_ip | local_ip;
		dhcps_lease.end_ip.addr = softap_ip | (local_ip + (DHCPS_MAX_LEASE << 24));
	}
#if DHCPS_DEBUG
	os_printf("start_ip = 0x%x, end_ip = 0x%x, enable=%u\n",dhcps_lease.start_ip, dhcps_lease.end_ip, dhcps_lease_flag);
#endif
}
///////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR dhcps_start(struct ip_info *info)
{
	struct netif * apnetif = (struct netif *)eagle_lwip_getif(0x01);
    
	if(apnetif->dhcps_pcb != NULL) {
        udp_remove(apnetif->dhcps_pcb);
    }
    if(dhcps_lease_time == 0) dhcps_lease_time = DHCPS_LEASE_TIME_DEF;

	pcb_dhcps = udp_new();
	if (pcb_dhcps == NULL || info ==NULL) {
		os_printf("dhcps_start(): could not obtain pcb\n");
	}

	apnetif->dhcps_pcb = pcb_dhcps;

	IP4_ADDR(&broadcast_dhcps, 255, 255, 255, 255);

	server_address = info->ip;
	wifi_softap_init_dhcps_lease(server_address.addr);
	client_address_plus.addr = dhcps_lease.start_ip.addr;

	udp_bind(pcb_dhcps, IP_ADDR_ANY, DHCPS_SERVER_PORT);
	udp_recv(pcb_dhcps, handle_dhcp, NULL);
#if DHCPS_DEBUG
	os_printf("dhcps:dhcps_start->udp_recv function Set a receive callback handle_dhcp for UDP_PCB pcb_dhcps\n");
#endif
		
}

void ICACHE_FLASH_ATTR dhcps_stop(void)
{
	struct netif * apnetif = (struct netif *)eagle_lwip_getif(0x01);

	udp_disconnect(pcb_dhcps);

	if(apnetif->dhcps_pcb != NULL) {
        udp_remove(apnetif->dhcps_pcb);
        apnetif->dhcps_pcb = NULL;
    }

	//udp_remove(pcb_dhcps);
	list_node *pnode = NULL;
	list_node *pback_node = NULL;
	pnode = plist;
	while (pnode != NULL) {
		pback_node = pnode;
		pnode = pback_node->pnext;
		node_remove_from_list(&plist, pback_node);
		os_free(pback_node->pnode);
		pback_node->pnode = NULL;
		os_free(pback_node);
		pback_node = NULL;
	}
}

/******************************************************************************
 * FunctionName : wifi_softap_set_dhcps_lease
 * Description  : set the lease information of DHCP server
 * Parameters   : please -- Additional argument to set the lease information,
 * 							Little-Endian.
 * Returns      : true or false
*******************************************************************************/
bool ICACHE_FLASH_ATTR wifi_softap_set_dhcps_lease(struct dhcps_lease *please)
{
	if (please == NULL || (please->end_ip.addr - please->start_ip.addr > (DHCPS_MAX_LEASE << 24)))
			return false;
	dhcps_lease.start_ip.addr = please->start_ip.addr;
	dhcps_lease.end_ip.addr = please->end_ip.addr;
	dhcps_lease_flag = true;
	return true;
}

/******************************************************************************
 * FunctionName : wifi_softap_get_dhcps_lease
 * Description  : get the lease information of DHCP server
 * Parameters   : please -- Additional argument to get the lease information,
 * 							Little-Endian.
 * Returns      : true or false
*******************************************************************************/
bool ICACHE_FLASH_ATTR wifi_softap_get_dhcps_lease(struct dhcps_lease *please)
{
	if (NULL == please)
		return false;
	please->start_ip.addr = dhcps_lease.start_ip.addr;
	please->end_ip.addr = dhcps_lease.end_ip.addr;
	return true;
}

static void ICACHE_FLASH_ATTR kill_oldest_dhcps_pool(void)
{
	list_node *pre = NULL, *p = NULL;
	list_node *minpre = NULL, *minp = NULL;
	struct dhcps_pool *pdhcps_pool = NULL, *pmin_pool = NULL;
	pre = plist;
	p = pre->pnext;
	minpre = pre;
	minp = p;
	while (p != NULL){
		pdhcps_pool = p->pnode;
		pmin_pool = minp->pnode;
		if (pdhcps_pool->lease_timer < pmin_pool->lease_timer){
			minp = p;
			minpre = pre;
		}
		pre = p;
		p = p->pnext;
	}
	minpre->pnext = minp->pnext;
	os_free(minp->pnode);
	minp->pnode = NULL;
	os_free(minp);
	minp = NULL;
}

void ICACHE_FLASH_ATTR dhcps_coarse_tmr(void)
{
	uint8 num_dhcps_pool = 0;
	list_node *pback_node = NULL;
	list_node *pnode = NULL;
	struct dhcps_pool *pdhcps_pool = NULL;
	pnode = plist;
	while (pnode != NULL) {
		pdhcps_pool = pnode->pnode;
		pdhcps_pool->lease_timer --;
		if (pdhcps_pool->lease_timer == 0){
			pback_node = pnode;
			pnode = pback_node->pnext;
			node_remove_from_list(&plist,pback_node);
			os_free(pback_node->pnode);
			pback_node->pnode = NULL;
			os_free(pback_node);
			pback_node = NULL;
		} else {
			pnode = pnode ->pnext;
			num_dhcps_pool ++;
		}
	}

	if (num_dhcps_pool >= MAX_STATION_NUM)
		kill_oldest_dhcps_pool();
}

bool ICACHE_FLASH_ATTR wifi_softap_set_dhcps_offer_option(uint8 level, void* optarg)
{
	bool offer_flag = true;
//	uint8 option = 0;
	if (optarg == NULL && wifi_softap_dhcps_status() == false)
		return false;

	if (level <= OFFER_START || level >= OFFER_END)
		return false;

	switch (level){
		case OFFER_ROUTER:
			offer = (*(uint8 *)optarg) & 0x01;
			offer_flag = true;
			break;
		default :
			offer_flag = false;
			break;
	}
	return offer_flag;
}

bool ICACHE_FLASH_ATTR wifi_softap_set_dhcps_lease_time(uint32 minute)
{
/*    uint8 opmode = wifi_get_opmode();

    if (opmode == STATION_MODE || opmode == NULL_MODE) {
        return false;
    }

    if (wifi_softap_dhcps_status() == DHCP_STARTED) {
        return false;
    }
*/
    if(minute == 0) {
        return false;
    }
    dhcps_lease_time = minute;
    return true;
}

bool ICACHE_FLASH_ATTR wifi_softap_reset_dhcps_lease_time(void)
{
/*    uint8 opmode = wifi_get_opmode();

    if (opmode == STATION_MODE || opmode == NULL_MODE) {
        return false;
    }

    if (wifi_softap_dhcps_status() == DHCP_STARTED) {
        return false;
    } */
    dhcps_lease_time = DHCPS_LEASE_TIME_DEF;
    return true;
}

uint32 ICACHE_FLASH_ATTR wifi_softap_get_dhcps_lease_time(void) // minute
{
    return dhcps_lease_time;
}

#endif // USE_OPEN_DHCPS
