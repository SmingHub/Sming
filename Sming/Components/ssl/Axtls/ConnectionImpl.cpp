/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ConnectionImpl.cpp
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 * Contains code from compatibility layer https://github.com/attachix/lwirax.git
 * so we can work directly with axTLS.
 *
 * Compatibility for AxTLS with LWIP raw tcp mode (http://lwip.wikia.com/wiki/Raw/TCP)
 *
 *  Created on: Jan 15, 2016
 *      Author: Slavey Karadzhov
 *
 ****/

/*
 */
#include "ConnectionImpl.h"

namespace Ssl
{
int ConnectionImpl::read(pbuf* encrypted, pbuf*& decrypted)
{
	assert(ssl != nullptr);
	assert(encrypted != nullptr);
	assert(tcp_pbuf == nullptr);

	if(encrypted->tot_len == 0) {
		// nothing to read
		return 0;
	}

	tcp_pbuf = encrypted;
	pbuf_offset = 0;

	int total_bytes = 0;
	uint8_t* total_read_buffer = nullptr;
	do {
		uint8_t* read_buffer = nullptr;
		int read_bytes = ssl_read(ssl, &read_buffer);
		debug_d("axl_ssl_read: Read bytes: %d", read_bytes);
		if(read_bytes < SSL_OK) {
			/* An error has occurred. Give it back for further processing */
			if(total_bytes == 0) {
				// Nothing is read so far -> give back the error
				total_bytes = read_bytes;
			} else {
				// We already have read some data -> deliver it back and silence the error for now..
				debug_w("axl_ssl_read: Silently ignoring SSL error %d", read_bytes);
			}

			break;
		}

		if(read_bytes == 0) {
			continue;
		}

		if(total_read_buffer == nullptr) {
			total_read_buffer = (uint8_t*)malloc(read_bytes);
		} else {
			debug_i("axl_ssl_read: Got more than one SSL packet inside one TCP packet");
			uint8_t* new_buffer = (uint8_t*)realloc(total_read_buffer, total_bytes + read_bytes);
			if(new_buffer == nullptr) {
				free(total_read_buffer);
				total_read_buffer = nullptr;
			} else {
				total_read_buffer = new_buffer;
			}
		}

		if(total_read_buffer == nullptr) {
			debug_e("axl_ssl_read: Unable to allocate additional %d bytes", read_bytes);
			total_bytes = -1;
			break;
		}

		memcpy(total_read_buffer + total_bytes, read_buffer, read_bytes);
		total_bytes += read_bytes;
	} while(encrypted->tot_len - pbuf_offset > 0);

	if(total_bytes > 0) {
		// put the decrypted data in a brand new pbuf
		decrypted = pbuf_alloc(PBUF_TRANSPORT, total_bytes, PBUF_RAM);
		if(decrypted != nullptr) {
			memcpy(decrypted->payload, total_read_buffer, total_bytes);
		} else {
			debug_e("Unable to allocate pbuf memory. Required %d. Check MEM_SIZE in your lwipopts.h file and "
					"increase if needed.",
					total_bytes);
			total_bytes = -1;
		}
		free(total_read_buffer);
	}

	tcp_pbuf = nullptr;

	return total_bytes;
}

int ConnectionImpl::write(const uint8_t* data, size_t length)
{
	int expected = calcWriteSize(length);
	u16_t available = tcp ? tcp_sndbuf(tcp) : 0;
	debug_d("SSL: Expected: %d, Available: %u", expected, available);
	if(expected < 0 || int(available) < expected) {
		return ERR_MEM;
	}

	int written = ssl_write(ssl, data, length);
	debug_d("SSL: Write len: %d, Written: %d", length, written);
	if(written < 0) {
		debug_e("SSL: Write Error: %d", written);
		return written;
	}

	return ERR_OK;
}

/*
 * Lower Level LWIP RAW functions
 */

/*
 * The LWIP tcp raw version of the SOCKET_WRITE(A, B, C)
 */
extern "C" int ax_port_write(int clientfd, uint8_t* buf, uint16_t bytes_needed)
{
	assert(clientfd != 0);

	auto connection = reinterpret_cast<ConnectionImpl*>(clientfd);
	return connection->port_write(buf, bytes_needed);
}

int ConnectionImpl::port_write(uint8_t* buf, uint16_t bytes_needed)
{
	assert(tcp != nullptr);

	if(buf == nullptr || bytes_needed == 0) {
		debug_w("ax_port_write: Return Zero.");
		return 0;
	}

	int tcp_len = 0;
	if(tcp_sndbuf(tcp) < bytes_needed) {
		tcp_len = tcp_sndbuf(tcp);
		if(tcp_len == 0) {
			tcp_output(tcp);
			debug_e("ax_port_write: The send buffer is full! We have problem.");
			return 0;
		}

	} else {
		tcp_len = bytes_needed;
	}

	if(tcp_len > 2 * tcp->mss) {
		tcp_len = 2 * tcp->mss;
	}

	err_t err;
	while((err = tcp_write(tcp, buf, tcp_len, TCP_WRITE_FLAG_COPY)) == ERR_MEM) {
		debug_e("ax_port_write: Not enough memory to write data with length: %d (%d)", tcp_len, bytes_needed);
		tcp_len /= 2;
		if(tcp_len <= 1) {
			tcp_len = 0;
			break;
		}
	}

	if(err == ERR_OK) {
		debug_d("ax_port_write: send_raw_packet length %d(%d)", tcp_len, bytes_needed);
		err = tcp_output(tcp);
		if(err != ERR_OK) {
			debug_e("ax_port_write: tcp_output got err: %d", err);
		}
	} else {
		debug_e("ax_port_write: Got error: %d", err);
	}

	return tcp_len;
}

/*
 * The LWIP tcp raw version of the SOCKET_READ(A, B, C)
 */
extern "C" int ax_port_read(int clientfd, uint8_t* buf, int bytes_needed)
{
	assert(clientfd != 0);

	auto connection = reinterpret_cast<ConnectionImpl*>(clientfd);
	return connection->port_read(buf, bytes_needed);
}

int ConnectionImpl::port_read(uint8_t* buf, int bytes_needed)
{
	if(tcp_pbuf == nullptr || tcp_pbuf->tot_len == 0) {
		debug_w("ax_port_read: Nothing to read?! May be the connection needs resetting?");
		return 0;
	}

	auto read_buf = new uint8_t[tcp_pbuf->len + 1];
	if(read_buf == nullptr) {
		debug_e("ax_port_read: Out of memory (%d bytes required)", tcp_pbuf->len + 1);
		return 0;
	}

	unsigned recv_len = pbuf_copy_partial(tcp_pbuf, read_buf, bytes_needed, pbuf_offset);
	pbuf_offset += recv_len;
	if(recv_len != 0) {
		memcpy(buf, read_buf, recv_len);
	}

	if(int(recv_len) < bytes_needed) {
		debug_d("ax_port_read: Bytes needed: %d, Bytes read: %d", bytes_needed, recv_len);
	}

	delete read_buf;

	return recv_len;
}

} // namespace Ssl
