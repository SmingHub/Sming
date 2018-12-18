#ifndef ESPCONN_SSL_CLIENT_H
#define ESPCONN_SSL_CLIENT_H

#include "ssl/ssl_ssl.h"
#include "ssl/ssl_tls1.h"

#include "lwip/app/espconn.h"

#define SSL_KEEP_INTVL  1000
#define SSL_KEEP_CNT	3
#define SSL_KEEP_IDLE	60000

#define  ssl_keepalive_enable(pcb)   ((pcb)->so_options |= SOF_KEEPALIVE)
#define  ssl_keepalive_disable(pcb)   ((pcb)->so_options &= ~SOF_KEEPALIVE)

typedef struct {
	char *buffer;
	int length;
	int start;
	int end;
}EspRingBuffer;

int EspRingBuffer_full(EspRingBuffer *buffer);
int EspRingBuffer_empty(EspRingBuffer *buffer);
int EspRingBuffer_available_data(EspRingBuffer *buffer);
int EspRingBuffer_available_space(EspRingBuffer *buffer);

#define EspRingBuffer_available_data(B)		(((B)->end + 1) % (B)->length - (B)->start -1)
#define EspRingBuffer_available_space(B)	((B)->length - (B)->end -1)
#define EspRingBuffer_full(B)				(EspRingBuffer_available_data((B))- (B)->length == 0)
#define EspRingBuffer_empty(B)				(EspRingBuffer_available_data((B)) == 0)
#define EspRingBuffer_starts_at(B)			((B)->buffer + (B)->start)
#define EspRingBuffer_ends_at(B)			((B)->buffer + (B)->end)
#define EspRingBuffer_commit_read(B, A)		((B)->start = ((B)->start + (A)) % (B)->length)
#define EspRingBuffer_commit_write(B, A)	((B)->end = ((B)->end + (A)) % (B)->length)

typedef struct _SSL_RING{
	EspRingBuffer *RingBuffer;
	uint16 RemainLength;
}SSL_RING;

typedef struct _ssl_msg {
    SSL_CTX *ssl_ctx;
    SSL *ssl;
    SSL_RING SSLRing;
    bool quiet;
    bool SentFnFlag;
    u16_t pkt_length;
} ssl_msg;

typedef struct _ssl_sector{
	uint32 sector;
	bool flag;
}ssl_sector;

struct ssl_packet{
	uint8* pbuffer;
	uint16 buffer_size;
	ssl_sector cert_ca_sector;
	ssl_sector cert_req_sector;
};

typedef struct _ssl_opt {
	struct ssl_packet server;
	struct ssl_packet client;
	uint8 type;
}ssl_opt;

enum {
	ESPCONN_IDLE = 0,
	ESPCONN_CLIENT,
	ESPCONN_SERVER,
	ESPCONN_BOTH,
	ESPCONN_MAX
};

enum {
	SIG_ESPCONN_TLS_ERRER = 0x3B
};

enum {
	ESPCONN_CERT_REQ,
	ESPCONN_CERT_AUTH
};

typedef struct _file_head{
	char file_name[32];
	uint16_t file_length;
}file_head;

typedef struct _file_param{
	file_head file_head;
	int32 file_offerset;
}file_param;

#define ESPCONN_SECURE_MAX_SIZE 8192
#define ESPCONN_SECURE_DEFAULT_HEAP 0x3800
#define ESPCONN_SECURE_DEFAULT_SIZE RT_MAX_PLAIN_LENGTH+RT_EXTRA
#define ESPCONN_HANDSHAKE_TIMEOUT 0x3C

#define espconn_TlsTaskPrio		25

extern ssl_opt ssl_option;

/******************************************************************************
 * FunctionName : sslserver_start
 * Description  : Initialize the server: set up a listen PCB and bind it to 
 *                the defined port
 * Parameters   : espconn -- the espconn used to build client
 * Returns      : none
*******************************************************************************/

extern sint8 espconn_ssl_server(struct espconn *espconn);

/******************************************************************************
 * FunctionName : espconn_ssl_client
 * Description  : Initialize the client: set up a connect PCB and bind it to 
 *                the defined port
 * Parameters   : espconn -- the espconn used to build client
 * Returns      : none
*******************************************************************************/

extern sint8 espconn_ssl_client(struct espconn *espconn);

/******************************************************************************
 * FunctionName : espconn_ssl_write
 * Description  : sent data for client or server
 * Parameters   : void *arg -- client or server to send
 * 				  uint8* psent -- Data to send
 *                uint16 length -- Length of data to send
 * Returns      : none
*******************************************************************************/

extern void espconn_ssl_sent(void *arg, uint8 *psent, uint16 length);

/******************************************************************************
 * FunctionName : espconn_ssl_disconnect
 * Description  : A new incoming connection has been disconnected.
 * Parameters   : espconn -- the espconn used to disconnect with host
 * Returns      : none
*******************************************************************************/

extern void espconn_ssl_disconnect(espconn_msg *pdis);

/******************************************************************************
 * FunctionName : espconn_secure_get_size
 * Description  : get buffer size for client or server
 * Parameters   : level -- set for client or server
 *				  1: client,2:server,3:client and server
 * Returns      : buffer size for client or server
*******************************************************************************/

extern sint16 espconn_secure_get_size(uint8 level);

#endif

