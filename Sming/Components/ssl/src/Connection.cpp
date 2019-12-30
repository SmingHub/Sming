/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Connection.cpp
 *
 ****/

#include <SslDebug.h>
#include <Network/Ssl/Context.h>
#include <Print.h>

namespace Ssl
{
size_t Connection::printTo(Print& p) const
{
	size_t n = 0;
	auto cert = getCertificate();
	if(cert != nullptr) {
		n += cert->printTo(p);
	}
	n += p.println(_F("SSL Connection Information:"));
	n += p.print(_F("  Cipher:       "));
	n += p.println(getCipherSuiteName(getCipherSuite()));
	n += p.print(_F("  Session ID:   "));
	n += p.println(getSessionId());
	return n;
}

int Connection::writeTcpData(uint8_t* data, size_t length)
{
	if(data == nullptr || length == 0) {
		debug_w("writeTcpData: Return Zero.");
		return 0;
	}

	//	debug_hex(INFO, "WRITE", data, length);

	size_t tcp_len = tcp_sndbuf(tcp);
	if(tcp_len < length) {
		if(tcp_len == 0) {
			tcp_output(tcp);
			debug_e("writeTcpData: The send buffer is full! We have problem.");
			return 0;
		}
	} else {
		tcp_len = length;
	}

	if(tcp_len > 2 * tcp->mss) {
		tcp_len = 2 * tcp->mss;
	}

	err_t err;
	while((err = tcp_write(tcp, data, tcp_len, TCP_WRITE_FLAG_COPY)) == ERR_MEM) {
		debug_e("writeTcpData: Not enough memory to write data with length: %d (%d)", tcp_len, length);
		tcp_len /= 2;
		if(tcp_len <= 1) {
			tcp_len = 0;
			break;
		}
	}

	if(err == ERR_OK) {
		debug_d("writeTcpData: length %d (%d)", tcp_len, length);
		err = tcp_output(tcp);
		if(err != ERR_OK) {
			debug_e("writeTcpData: tcp_output got err: %d", err);
		}
	} else {
		debug_e("writeTcpData: Got error: %d", err);
	}

	return tcp_len;
}

} // namespace Ssl
