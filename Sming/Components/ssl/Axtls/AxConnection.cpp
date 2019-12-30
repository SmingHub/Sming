/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxConnection.cpp
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

#include "AxConnection.h"
#include "AxContext.h"
#include <Network/Ssl/Session.h>

namespace Ssl
{
int AxConnection::write(const uint8_t* data, size_t length)
{
	int required = ssl_calculate_write_length(ssl, length);
	if(required < 0) {
		return required;
	}

	int available = tcp_sndbuf(tcp);
	if(available < required) {
#ifdef SSL_DEBUG
		debug_i("SSL: Required: %d, Available: %d", required, available);
#endif
		return SSL_NOT_OK;
	}

	int written = ssl_write(ssl, data, length);
#ifdef SSL_DEBUG
	debug_d("SSL: Write len: %d, Written: %d", length, written);
#endif
	if(written < 0) {
		debug_e("SSL: Write Error: %d", written);
	}

	return written;
}

int AxConnection::read(InputBuffer& input, uint8_t*& output)
{
	bool connected = isHandshakeDone();
	this->input = &input;
	int readBytes = ssl_read(ssl, &output);
	this->input = nullptr;
	if(!connected && isHandshakeDone()) {
		auto& session = context.getSession();
		if(session.getConnection() != nullptr) {
			if(!session.validateCertificate()) {
				session.handshakeComplete(false);
				return SSL_ERROR_BAD_CERTIFICATE;
			}
			session.handshakeComplete(true);
		}
	}

	if(readBytes == SSL_CLOSE_NOTIFY) {
		readBytes = 0;
	}

	return readBytes;
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

	auto connection = reinterpret_cast<AxConnection*>(clientfd);
	return connection->writeTcpData(buf, bytes_needed);
}

/*
 * The LWIP tcp raw version of the SOCKET_READ(A, B, C)
 */
extern "C" int ax_port_read(int clientfd, uint8_t* buf, int bytes_needed)
{
	assert(clientfd != 0);

	auto connection = reinterpret_cast<AxConnection*>(clientfd);
	return connection->readTcpData(buf, bytes_needed);
}

} // namespace Ssl
