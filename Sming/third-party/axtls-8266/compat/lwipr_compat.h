/*
 * Compatibility for AxTLS with LWIP raw tcp mode (http://lwip.wikia.com/wiki/Raw/TCP)
 *
 *  Created on: Jan 15, 2016
 *      Author: Slavey Karadzhov
 */

#ifndef LWIPR_COMPAT_H
#define LWIPR_COMPAT_H

/*
 * All those functions will run only if LWIP tcp raw mode is used
 */
#if LWIP_RAW==1

#ifdef __cplusplus
extern "C" {
#endif

#include "lwipr_platform.h"
#include "ssl/ssl.h"
#include "ssl/tls1.h"

#define ERR_AXL_INVALID_SSL -101
#define ERR_AXL_INVALID_TCP -102
#define ERR_AXL_INVALID_CLIENTFD -103
#define ERR_AXL_INVALID_CLIENTFD_DATA -104
#define ERR_AXL_INVALID_PBUF -105

#define SOCKET_READ(A, B, C) 	ax_port_read(A, B, C)
#define SOCKET_WRITE(A, B, C) 	ax_port_write(A, B, C)

/**
 * Define watchdog function to be called during CPU intensive operations.
 */
#ifndef WATCHDOG_RESET
	#define WATCHDOG_RESET()
#endif

typedef struct {
	struct tcp_pcb *tcp;
	struct pbuf *tcp_pbuf;
	int pbuf_offset;
} AxlTcpData;


typedef struct {
  int size;      /* slots used so far */
  int capacity;  /* total available slots */
  AxlTcpData *data;     /* array of TcpData objects */
} AxlTcpDataArray;

/*
 * High Level Functions - these are the ones that should be used directly
 */

int axl_init(int capacity);
int axl_append(struct tcp_pcb *tcp);
int axl_free(struct tcp_pcb *tcp);
void axl_destroy();

#define axl_ssl_write(A, B, C) ssl_write(A, B, C)
int axl_ssl_read(SSL *sslObj, struct tcp_pcb *tcp, struct pbuf *pin, struct pbuf **pout);

/*
 * Lower Level Socket Functions - used internally from axTLS
 */

int ax_port_write(int clientfd, uint8_t *buf, uint16_t bytes_needed);
int ax_port_read(int clientfd, uint8_t *buf, int bytes_needed);

/*
 * Lower Level Utility functions
 */
int ax_fd_init(AxlTcpDataArray *vector, int capacity);
int ax_fd_append(AxlTcpDataArray *vector, struct tcp_pcb *tcp);
AxlTcpData* ax_fd_get(AxlTcpDataArray *vector, int index);
int ax_fd_getfd(AxlTcpDataArray *vector, struct tcp_pcb *tcp);
void ax_fd_set(AxlTcpDataArray *vector, int index, struct tcp_pcb *tcp);
void ax_fd_double_capacity_if_full(AxlTcpDataArray *vector);
void ax_fd_free(AxlTcpDataArray *vector);


#ifdef __cplusplus
}
#endif

#endif /* LWIP_RAW==1 */

#endif /* LWIPR_COMPAT_H */
