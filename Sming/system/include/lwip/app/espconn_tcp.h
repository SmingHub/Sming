#ifndef __ESPCONN_TCP_H__
#define __ESPCONN_TCP_H__

#include "lwipopts.h"
#ifdef USE_ESPCONN

#ifndef ESPCONN_TCP_DEBUG
#define ESPCONN_TCP_DEBUG LWIP_DBG_OFF
#endif

#include "lwip/app/espconn.h"

#ifndef ESPCONN_TCP_TIMER
#define ESPCONN_TCP_TIMER 40
#endif

#define  espconn_keepalive_enable(pcb)   ((pcb)->so_options |= SOF_KEEPALIVE)
#define  espconn_keepalive_disable(pcb)   ((pcb)->so_options &= ~SOF_KEEPALIVE)

/******************************************************************************
 * FunctionName : espconn_tcp_write
 * Description  : write the packet which in the active connection's list.
 * Parameters   : arg -- the node pointer which reverse the packet
 * Returns      : ESPCONN_MEM: memory error
 * 				  ESPCONN_OK:have enough space for write packet
*******************************************************************************/
err_t ICACHE_FLASH_ATTR espconn_tcp_write(void *arg);

/******************************************************************************
 * FunctionName : espconn_tcp_delete
 * Description  : delete the server: delete a listening PCB and free it
 * Parameters   : pdeletecon -- the espconn used to delete a server
 * Returns      : none
*******************************************************************************/
sint8 ICACHE_FLASH_ATTR espconn_tcp_delete(struct espconn *pdeletecon);

/******************************************************************************
 * FunctionName : espconn_kill_oldest_pcb
 * Description  : A oldest incoming connection has been killed.
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

extern void espconn_kill_oldest_pcb(void);

/******************************************************************************
 * FunctionName : espconn_tcp_disconnect
 * Description  : A new incoming connection has been disconnected.
 * Parameters   : espconn -- the espconn used to disconnect with host
 * Returns      : none
*******************************************************************************/

extern void espconn_tcp_disconnect(espconn_msg *pdiscon);

/******************************************************************************
 * FunctionName : espconn_tcp_client
 * Description  : Initialize the client: set up a connect PCB and bind it to 
 *                the defined port
 * Parameters   : espconn -- the espconn used to build client
 * Returns      : none
*******************************************************************************/

extern sint8 espconn_tcp_client(struct espconn* espconn);

/******************************************************************************
 * FunctionName : espconn_tcp_server
 * Description  : Initialize the server: set up a listening PCB and bind it to 
 *                the defined port
 * Parameters   : espconn -- the espconn used to build server
 * Returns      : none
*******************************************************************************/

extern sint8 espconn_tcp_server(struct espconn *espconn);

#endif // USE_ESPCONN

#endif /* __CLIENT_TCP_H__ */

