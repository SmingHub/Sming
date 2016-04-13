/*
 * Compatibility for AxTLS with LWIP raw tcp mode (http://lwip.wikia.com/wiki/Raw/TCP)
 *
 *  Created on: Jan 15, 2016
 *      Author: Slavey Karadzhov
 */
#include "lwipr_compat.h"

AxlTcpDataArray axlFdArray;

#include <stdlib.h>

/* High Level "public" functions */

/**
 * Function that should be called once we are ready to use the axTLS - LWIP raw compatibility
 */
void axl_init(int capacity) {
	ax_fd_init(&axlFdArray, capacity);
}

/**
 * Appends a tcp to the internal array. Returns client file descriptor
 */
int axl_append(struct tcp_pcb *tcp) {
	return ax_fd_append(&axlFdArray, tcp);
}

/**
 * Frees  the internal mapping from this tcp. Returns the number of occurrences of the tcp
 */
int axl_free(struct tcp_pcb *tcp) {
	int i;
	int occurances = 0;

	if(tcp == NULL) {
		return 0;
	}

	AxlTcpDataArray *vector = &axlFdArray;
	AXL_DEBUG_PRINT("axl_free: Freeing %d tcp item", vector->size);
	for (i = 0; i < vector->size; i++) {
		if (vector->data[i].tcp == tcp) {
			if(vector->data[i].tcp_pbuf != NULL) {
//				pbuf_free(vector->data[i].tcp_pbuf);
				vector->data[i].tcp_pbuf = NULL;
			}
			vector->data[i].tcp = NULL;
			vector->data[i].pbuf_offset = 0;
			occurances++;
		}
	}

	return occurances;
}

/**
 * Reads data from the SSL over TCP stream. Returns decrypted data.
 * @param SSL *sslObj
 * @param uint8_t **in_data - pointer to the decrypted incoming data, or NULL if nothing was read
 * @param void *arg - possible arguments passed to the tcp raw layer during initialization
 * @param tcp_pcb *tcp - pointer to the raw tcp object
 * @param pbuf *pin - pointer to the buffer with the encrypted TCP packet data
 * - the freeing of the pbuf should be done separately
 * - the confirmation for the received bytes should be done separately
 * @param pbuf **pout - pointer to the buffer with the decrypted TCP packet data
 * - the freeing of the pbuf should be done separately
 *
 * @return int
 * 			0 - when everything is fine but there are no symbols to process yet
 * 			< 0 - when there is an error
 * 			> 0 - the length of the clear text characters that were read
 */
int axl_ssl_read(SSL *ssl, struct tcp_pcb *tcp, struct pbuf *pin, struct pbuf **pout) {
	int read_bytes = 0;
	int total_bytes = 0;
	int clientfd = -1;
	uint8_t *read_buffer = NULL;
	uint8_t *total_read_buffer = NULL;
	AxlTcpData* data = NULL;

	if (ssl == NULL) {
		AXL_DEBUG_PRINT("axl_ssl_read: SSL is null\n");
		return ERR_AXL_INVALID_SSL;
	}

	if(pin == NULL) {
		AXL_DEBUG_PRINT("axl_ssl_read: PBUF is null\n");
		return ERR_AXL_INVALID_PBUF;
	}

	if(pin->tot_len == 0) {
		// nothing to read
		return 0;
	}

	clientfd = ax_fd_getfd(&axlFdArray, tcp);
	if(clientfd == -1) {
		AXL_DEBUG_PRINT("axl_ssl_read: ClientFD not found\n");
		return ERR_AXL_INVALID_CLIENTFD;
	}

	data = ax_fd_get(&axlFdArray, clientfd);
	if(data == NULL) {
		AXL_DEBUG_PRINT("axl_ssl_read: ClientFD data not found\n");
		return ERR_AXL_INVALID_CLIENTFD_DATA;
	}

	data->tcp_pbuf = pin;
	data->pbuf_offset = 0;

	do {
		WATCHDOG_RESET();
		read_bytes = ssl_read(ssl, &read_buffer);
		AXL_DEBUG_PRINT("axl_ssl_read: Read bytes: %d\n", read_bytes);
		if(read_bytes < SSL_OK) {
			/* An error has occurred. Give it back for further processing */
			if(total_bytes == 0) {
				// Nothing is read so far -> give back the error
				total_bytes = read_bytes;
			}
			else {
				// We already have read some data -> deliver it back
				// and silence the error for now..
				AXL_DEBUG_PRINT("axl_ssl_read: Silently ignoring SSL error %d\n", read_bytes);
			}

			break;
		}
		else if (read_bytes > 0 ){
			if(total_read_buffer == NULL) {
				total_read_buffer = (uint8_t *)malloc(read_bytes);
			}
			else {
				AXL_DEBUG_PRINT("axl_ssl_read: Got more than one SSL packet inside one TCP packet\n");
				total_read_buffer = (uint8_t *)realloc(total_read_buffer, total_bytes + read_bytes);
			}

			if(total_read_buffer == NULL) {
				AXL_DEBUG_PRINT("axl_ssl_read: Unable to allocate additional %d bytes", read_bytes);
				while(1) {}
			}

			memcpy(total_read_buffer + total_bytes, read_buffer, read_bytes);
		}
		total_bytes+= read_bytes;
	} while (pin->tot_len - data->pbuf_offset > 0);

	if(total_bytes > 0) {
		// put the decrypted data in a brand new pbuf
		*pout = pbuf_alloc(PBUF_TRANSPORT, total_bytes, PBUF_RAM);
		memcpy((*pout)->payload, total_read_buffer, total_bytes);
		free(total_read_buffer);
	}

	return total_bytes;
}

/*
 * Lower Level LWIP RAW functions
 */

/*
 * The LWIP tcp raw version of the SOCKET_WRITE(A, B, C)
 */
int ax_port_write(int clientfd, uint8_t *buf, uint16_t bytes_needed) {
	AxlTcpData *data = NULL;
	int tcp_len = 0;
	err_t err = ERR_OK;

	data = ax_fd_get(&axlFdArray, clientfd);
	if(data == NULL) {
		AXL_DEBUG_PRINT("ax_port_write: Invalid ClientFD.\n");
		return ERR_AXL_INVALID_CLIENTFD;
	}

	if (data == NULL || data->tcp == NULL || buf == NULL || bytes_needed == 0) {
		AXL_DEBUG_PRINT("ax_port_write: Return Zero.\n");
		return 0;
	}

	if (tcp_sndbuf(data->tcp) < bytes_needed) {
		tcp_len = tcp_sndbuf(data->tcp);
		if(tcp_len == 0) {
			err = tcp_output(data->tcp);
			AXL_DEBUG_PRINT("ax_port_write: The send buffer is full! We have problem.\n");
			return 0;
		}

	} else {
		tcp_len = bytes_needed;
	}

	if (tcp_len > 2 * data->tcp->mss) {
		tcp_len = 2 * data->tcp->mss;
	}

	do {
		err = tcp_write(data->tcp, buf, tcp_len, TCP_WRITE_FLAG_COPY);
		if(err < SSL_OK) {
			AXL_DEBUG_PRINT("ax_port_write: Got error: %d\n", err);
		}

		if (err == ERR_MEM) {
			AXL_DEBUG_PRINT("ax_port_write: Not enough memory to write data with length: %d (%d)\n", tcp_len, bytes_needed);
			tcp_len /= 2;
		}
	} while (err == ERR_MEM && tcp_len > 1);
	AXL_DEBUG_PRINT("ax_port_write: send_raw_packet length %d(%d)\n", tcp_len, bytes_needed);
	if (err == ERR_OK) {
		err = tcp_output(data->tcp);
		if(err != ERR_OK) {
			AXL_DEBUG_PRINT("ax_port_write: tcp_output got err: %d\n", err);
		}
	}

	return tcp_len;
}

/*
 * The LWIP tcp raw version of the SOCKET_READ(A, B, C)
 */
int ax_port_read(int clientfd, uint8_t *buf, int bytes_needed) {
	AxlTcpData *data = NULL;
	uint8_t *read_buf = NULL;
	uint8_t *pread_buf = NULL;
	u16_t recv_len = 0;

	data = ax_fd_get(&axlFdArray, clientfd);
	if (data == NULL) {
		return ERR_AXL_INVALID_CLIENTFD_DATA;
	}

	if(data->tcp_pbuf == NULL || data->tcp_pbuf->tot_len == 0) {
		AXL_DEBUG_PRINT("ax_port_read: Nothing to read?! May be the connection needs resetting?\n");
		return 0;
	}

	read_buf =(uint8_t*)calloc(data->tcp_pbuf->len + 1, sizeof(uint8_t));
	pread_buf = read_buf;
	if (pread_buf != NULL){
		recv_len = pbuf_copy_partial(data->tcp_pbuf, read_buf, bytes_needed, data->pbuf_offset);
		data->pbuf_offset += recv_len;
	}

	if (recv_len != 0) {
		memcpy(buf, read_buf, recv_len);
	}

	if(bytes_needed < recv_len) {
		AXL_DEBUG_PRINT("ax_port_read: Bytes needed: %d, Bytes read: %d\n", bytes_needed, recv_len);
	}

	free(pread_buf);
	pread_buf = NULL;

	return recv_len;
}

int ax_get_file(const char *filename, uint8_t **buf) {
    *buf = 0;
    return 0;
}

/*
 * Utility functions
 */
void ax_fd_init(AxlTcpDataArray *vector, int capacity) {
	vector->size = 0;
	vector->capacity = capacity;
	vector->data = (AxlTcpData*) malloc(sizeof(AxlTcpData) * vector->capacity);
}

int ax_fd_append(AxlTcpDataArray *vector, struct tcp_pcb *tcp) {
	int index;

	ax_fd_double_capacity_if_full(vector);
	index = vector->size++;
	vector->data[index].tcp = tcp;
	vector->data[index].tcp_pbuf = NULL;
	vector->data[index].pbuf_offset = 0;

	return index;
}

AxlTcpData* ax_fd_get(AxlTcpDataArray *vector, int index) {
	if (index >= vector->size || index < 0) {
		AXL_DEBUG_PRINT("ax_fd_get: Index %d out of bounds for vector of size %d\n", index, vector->size);
		return NULL;
	}
	return &(vector->data[index]);
}

int ax_fd_getfd(AxlTcpDataArray *vector, struct tcp_pcb *tcp) {
	int i;
	for (i = 0; i < vector->size; i++) {
		if (vector->data[i].tcp == tcp) {
			return i;
		}
	}

	return -1;
}

void ax_fd_set(AxlTcpDataArray *vector, int index, struct tcp_pcb *tcp) {
	AxlTcpData value;
	while (index >= vector->size) {
		ax_fd_append(vector, 0);
	}

	value.tcp = tcp;
	value.tcp_pbuf = NULL;
	value.pbuf_offset = 0;
	vector->data[index] = value;
}

void ax_fd_double_capacity_if_full(AxlTcpDataArray *vector) {
	if (vector->size >= vector->capacity) {
		vector->capacity *= 2;
		vector->data = (AxlTcpData*)realloc(vector->data, sizeof(AxlTcpData) * vector->capacity);
	}
}

void ax_fd_free(AxlTcpDataArray *vector) {
	free(vector->data);
}
