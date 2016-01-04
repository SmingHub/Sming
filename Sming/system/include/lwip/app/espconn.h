#ifndef __ESPCONN_LWIP_H__
#define __ESPCONN_LWIP_H__

#include "lwipopts.h"

#include "lwip/dns.h"

#ifdef USE_ESPCONN

#include "espconn.h"

#include "os_type.h"

#ifdef LWIP_DEBUG
#define espconn_printf(fmt, args...) m_printf(fmt,## args)
#else 
#define espconn_printf(fmt, args...)
#endif

/* Definitions for error constants. */

#define ESPCONN_RESP_TIMEOUT -29 /* ssl handshake no response*/

#define ESPCONN_SSL			0x01
#define ESPCONN_NORM		0x00

#define ESPCONN_STA			0x01
#define ESPCONN_AP			0x02
#define ESPCONN_AP_STA		0x03

#define STA_NETIF      0x00
#define AP_NETIF       0x01


typedef struct _espconn_buf{
	uint8 *payload;
	uint8 *punsent;
	uint16 unsent;
	uint16 len;
	uint16 tot_len;
	struct _espconn_buf *pnext;
} espconn_buf;

typedef struct _comon_pkt{
	void *pcb;
	int remote_port;
	uint8 remote_ip[4];
	uint32 local_port;
	uint32 local_ip;
	espconn_buf *pbuf;
	espconn_buf *ptail;
	uint8* ptrbuf;
	uint16 cntr;
	sint8  err;
	uint32 timeout;
	uint32 recv_check;
	uint8  pbuf_num;
	struct espconn_packet packet_info;
	bool write_flag;
	enum espconn_option espconn_opt;
}comon_pkt;

typedef struct _espconn_msg{
	struct espconn *pespconn;
	comon_pkt pcommon;
	uint8 count_opt;
	sint16_t hs_status;	//the status of the handshake
	void *preverse;
	void *pssl;
	struct _espconn_msg *pnext;

//***********Code for WIFI_BLOCK from upper**************
	uint8 recv_hold_flag;
	uint16 recv_holded_buf_Len;
}espconn_msg;


#define linkMax 15

#define   espconn_delay_disabled(espconn)  (((espconn)->pcommon.espconn_opt & ESPCONN_NODELAY) != 0)
#define   espconn_delay_enabled(espconn)  (((espconn)->pcommon.espconn_opt & ESPCONN_NODELAY) == 0)
#define   espconn_reuse_disabled(espconn)  (((espconn)->pcommon.espconn_opt & ESPCONN_REUSEADDR) != 0)
#define   espconn_copy_disabled(espconn)  (((espconn)->pcommon.espconn_opt & ESPCONN_COPY) != 0)
#define   espconn_copy_enabled(espconn)  (((espconn)->pcommon.espconn_opt & ESPCONN_COPY) == 0)
#define   espconn_keepalive_disabled(espconn)  (((espconn)->pcommon.espconn_opt & ESPCONN_KEEPALIVE) != 0)
#define   espconn_keepalive_enabled(espconn)  (((espconn)->pcommon.espconn_opt & ESPCONN_KEEPALIVE) == 0)

#define espconn_TaskPrio        26
#define espconn_TaskQueueLen    15

enum espconn_sig {
    SIG_ESPCONN_NONE,
    SIG_ESPCONN_ERRER,
    SIG_ESPCONN_LISTEN,
    SIG_ESPCONN_CONNECT,
    SIG_ESPCONN_WRITE,
    SIG_ESPCONN_SEND,
    SIG_ESPCONN_READ,
    SIG_ESPCONN_CLOSE
};

/******************************************************************************
 * FunctionName : espconn_pbuf_delete
 * Description  : remove the node from the active connection list
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR espconn_pbuf_delete(espconn_buf **phead, espconn_buf* pdelete);

/******************************************************************************
 * FunctionName : espconn_copy_partial
 * Description  : reconnect with host
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/

void espconn_copy_partial(struct espconn *pesp_dest, struct espconn *pesp_source);

/******************************************************************************
 * FunctionName : espconn_copy_partial
 * Description  : insert the node to the active connection list
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/

void espconn_list_creat(espconn_msg **phead, espconn_msg* pinsert);

/******************************************************************************
 * FunctionName : espconn_list_delete
 * Description  : remove the node from the active connection list
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/

void espconn_list_delete(espconn_msg **phead, espconn_msg* pdelete);

/******************************************************************************
 * FunctionName : espconn_find_connection
 * Description  : Initialize the server: set up a listening PCB and bind it to
 *                the defined port
 * Parameters   : espconn -- the espconn used to build server
 * Returns      : none
 *******************************************************************************/

bool espconn_find_connection(struct espconn *pespconn, espconn_msg **pnode);




/******************************************************************************
 * FunctionName : espconn_tcp_get_wnd
 * Description  : get the window size of simulatenously active TCP connections
 * Parameters   : none
 * Returns      : the number of TCP_MSS active TCP connections
*******************************************************************************/
extern uint8 espconn_tcp_get_wnd(void);

/******************************************************************************
 * FunctionName : espconn_tcp_set_max_con
 * Description  : set the window size simulatenously active TCP connections
 * Parameters   : num -- the number of TCP_MSS
 * Returns      : ESPCONN_ARG -- Illegal argument
 * 				  ESPCONN_OK  -- No error
*******************************************************************************/
extern sint8 espconn_tcp_set_wnd(uint8 num);


/******************************************************************************
 * FunctionName : espconn_tcp_get_max_retran
 * Description  : get the Maximum number of retransmissions of data active TCP connections
 * Parameters   : none
 * Returns      : the Maximum number of retransmissions
*******************************************************************************/
extern uint8 espconn_tcp_get_max_retran(void);

/******************************************************************************
 * FunctionName : espconn_tcp_set_max_retran
 * Description  : set the Maximum number of retransmissions of data active TCP connections
 * Parameters   : num -- the Maximum number of retransmissions
 * Returns      : result
*******************************************************************************/

extern sint8 espconn_tcp_set_max_retran(uint8 num);

/******************************************************************************
 * FunctionName : espconn_tcp_get_max_syn
 * Description  : get the Maximum number of retransmissions of SYN segments
 * Parameters   : none
 * Returns      : the Maximum number of retransmissions
*******************************************************************************/

extern uint8 espconn_tcp_get_max_syn(void);

/******************************************************************************
 * FunctionName : espconn_tcp_set_max_syn
 * Description  : set the Maximum number of retransmissions of SYN segments
 * Parameters   : num -- the Maximum number of retransmissions
 * Returns      : result
*******************************************************************************/

extern sint8 espconn_tcp_set_max_syn(uint8 num);

/******************************************************************************
 * FunctionName : espconn_tcp_set_buf_count
 * Description  : set the total number of espconn_buf on the unsent lists
 * Parameters   : espconn -- espconn to set the count
 * 				  num -- the total number of espconn_buf
 * Returns      : result
*******************************************************************************/

extern sint8 espconn_tcp_set_buf_count(struct espconn *espconn, uint8 num);

#endif // USE_ESPCONN

#endif

