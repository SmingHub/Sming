/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrConnection.cpp
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

/*
 */
#include <SslDebug.h>
#include "BrConnection.h"
#include <Network/Ssl/Session.h>
#include <FlashString/Array.hpp>

// Defined in ssl_engine.c
#define MAX_OUT_OVERHEAD 85
#define MAX_IN_OVERHEAD 325

namespace Ssl
{
int BrConnection::init(size_t bufferSize, bool bidi)
{
	DEFINE_FSTR_ARRAY_LOCAL(FS_suitesBasic, CipherSuite, CipherSuite::RSA_WITH_AES_128_CBC_SHA256,
							CipherSuite::RSA_WITH_AES_256_CBC_SHA256, CipherSuite::RSA_WITH_AES_128_CBC_SHA,
							CipherSuite::RSA_WITH_AES_256_CBC_SHA);

	DEFINE_FSTR_ARRAY_LOCAL(
		FS_suitesFull, CipherSuite, CipherSuite::ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
		CipherSuite::ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256, CipherSuite::ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
		CipherSuite::ECDHE_RSA_WITH_AES_128_GCM_SHA256, CipherSuite::ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
		CipherSuite::ECDHE_RSA_WITH_AES_256_GCM_SHA384, CipherSuite::ECDHE_ECDSA_WITH_AES_128_CCM,
		CipherSuite::ECDHE_ECDSA_WITH_AES_256_CCM, CipherSuite::ECDHE_ECDSA_WITH_AES_128_CCM_8,
		CipherSuite::ECDHE_ECDSA_WITH_AES_256_CCM_8, CipherSuite::ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
		CipherSuite::ECDHE_RSA_WITH_AES_128_CBC_SHA256, CipherSuite::ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
		CipherSuite::ECDHE_RSA_WITH_AES_256_CBC_SHA384, CipherSuite::ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
		CipherSuite::ECDHE_RSA_WITH_AES_128_CBC_SHA, CipherSuite::ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
		CipherSuite::ECDHE_RSA_WITH_AES_256_CBC_SHA, CipherSuite::ECDH_ECDSA_WITH_AES_128_GCM_SHA256,
		CipherSuite::ECDH_RSA_WITH_AES_128_GCM_SHA256, CipherSuite::ECDH_ECDSA_WITH_AES_256_GCM_SHA384,
		CipherSuite::ECDH_RSA_WITH_AES_256_GCM_SHA384, CipherSuite::ECDH_ECDSA_WITH_AES_128_CBC_SHA256,
		CipherSuite::ECDH_RSA_WITH_AES_128_CBC_SHA256, CipherSuite::ECDH_ECDSA_WITH_AES_256_CBC_SHA384,
		CipherSuite::ECDH_RSA_WITH_AES_256_CBC_SHA384, CipherSuite::ECDH_ECDSA_WITH_AES_128_CBC_SHA,
		CipherSuite::ECDH_RSA_WITH_AES_128_CBC_SHA, CipherSuite::ECDH_ECDSA_WITH_AES_256_CBC_SHA,
		CipherSuite::ECDH_RSA_WITH_AES_256_CBC_SHA, CipherSuite::RSA_WITH_AES_128_GCM_SHA256,
		CipherSuite::RSA_WITH_AES_256_GCM_SHA384, CipherSuite::RSA_WITH_AES_128_CCM, CipherSuite::RSA_WITH_AES_256_CCM,
		CipherSuite::RSA_WITH_AES_128_CCM_8, CipherSuite::RSA_WITH_AES_256_CCM_8,
		CipherSuite::RSA_WITH_AES_128_CBC_SHA256, CipherSuite::RSA_WITH_AES_256_CBC_SHA256,
		CipherSuite::RSA_WITH_AES_128_CBC_SHA, CipherSuite::RSA_WITH_AES_256_CBC_SHA,
		CipherSuite::ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA, CipherSuite::ECDHE_RSA_WITH_3DES_EDE_CBC_SHA,
		CipherSuite::ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA, CipherSuite::ECDH_RSA_WITH_3DES_EDE_CBC_SHA,
		CipherSuite::RSA_WITH_3DES_EDE_CBC_SHA);

	auto& FS_suites = FS_suitesFull;

	auto engine = getEngine();
	br_ssl_engine_set_versions(engine, BR_TLS10, BR_TLS12);

	LOAD_FSTR_ARRAY(suites, FS_suites);
	br_ssl_engine_set_suites(engine, (uint16_t*)suites, FS_suites.length());
	br_ssl_engine_set_default_rsavrfy(engine);
	br_ssl_engine_set_default_ecdsa(engine);

	// Set supported hash functions for the SSL engine
#define INSTALL_HASH(hash) br_ssl_engine_set_hash(engine, br_##hash##_ID, &br_##hash##_vtable);
	INSTALL_HASH(md5)
	INSTALL_HASH(sha1)
	INSTALL_HASH(sha224)
	INSTALL_HASH(sha256)
	INSTALL_HASH(sha384)
	INSTALL_HASH(sha512)
#undef INSTALL_HASH

	// Set the PRF implementations
	br_ssl_engine_set_prf10(engine, &br_tls10_prf);
	br_ssl_engine_set_prf_sha256(engine, &br_tls12_sha256_prf);
	br_ssl_engine_set_prf_sha384(engine, &br_tls12_sha384_prf);

	// Symmetric encryption
	br_ssl_engine_set_default_aes_cbc(engine);
	br_ssl_engine_set_default_aes_ccm(engine);
	br_ssl_engine_set_default_aes_gcm(engine);
	br_ssl_engine_set_default_des_cbc(engine);
	br_ssl_engine_set_default_chapol(engine);

	bufferSize += MAX_IN_OVERHEAD;
	if(bidi) {
		bufferSize += MAX_OUT_OVERHEAD;
	}
	debug_i("Using buffer size of %u bytes", bufferSize);
	delete[] buffer;
	buffer = new uint8_t[bufferSize];
	if(buffer == nullptr) {
		debug_e("Buffer allocation failed");
		return -BR_ERR_BAD_PARAM;
	}
	br_ssl_engine_set_buffer(engine, buffer, bufferSize, bidi);

	return BR_ERR_OK;
}

int BrConnection::read(InputBuffer& input, uint8_t*& output)
{
	int state = runUntil(input, BR_SSL_RECVAPP);
	if(state <= 0) {
		return state;
	}

	if((state & BR_SSL_RECVAPP) == 0) {
		return 0;
	}

	auto engine = getEngine();

	size_t len = 0;
	output = br_ssl_engine_recvapp_buf(engine, &len);
	debug_hex(DBG, "READ", output, len);
	br_ssl_engine_recvapp_ack(engine, len);
	return len;
}

int BrConnection::write(const uint8_t* data, size_t length)
{
	InputBuffer input(nullptr);
	int state = runUntil(input, BR_SSL_SENDAPP);
	if(state < 0) {
		return state;
	}

	if((state & BR_SSL_SENDAPP) == 0) {
		return -BR_ERR_BAD_STATE;
	}

	auto engine = getEngine();

	size_t available;
	auto buf = br_ssl_engine_sendapp_buf(engine, &available);
	if(available == 0) {
		debug_w("SSL: Send buffer full");
		return 0;
	}

	if(available < length) {
		debug_i("SSL: Required: %d, Available: %u", length, available);
		length = available;
	}

	memcpy(buf, data, length);
	br_ssl_engine_sendapp_ack(engine, length);
	br_ssl_engine_flush(engine, 0);

	/*
	 * Our data has been accepted so just let the SSL engine run so it can try to
	 * send some data.
	 * This can fail if the engine expects some receive data, so don't respond to
	 * the return value as this will get resolved on the next read operation.
	 */
	runUntil(input, BR_SSL_SENDAPP | BR_SSL_RECVAPP);
	return length;
}

int BrConnection::runUntil(InputBuffer& input, unsigned target)
{
	auto engine = getEngine();

	for(;;) {
		unsigned state = br_ssl_engine_current_state(engine);

		if(state & BR_SSL_CLOSED) {
			int err = br_ssl_engine_last_error(engine);
			debug_w("SSL CLOSED, last error = %d (%s), heap free = %u", err, getErrorString(err).c_str(),
					system_get_free_heap_size());
			return -err;
		}

		if(!handshakeDone && (state & BR_SSL_SENDAPP)) {
			handshakeDone = true;
			context.getSession().handshakeComplete(true);
			debug_i("Negotiated MFLN: %u", br_ssl_engine_get_mfln_negotiated(engine));
			continue;
		}

		/*
		 * If there is some record data to send, do it. This takes
		 * precedence over everything else.
		 */
		if(state & BR_SSL_SENDREC) {
			size_t len;
			auto buf = br_ssl_engine_sendrec_buf(engine, &len);
			int wlen = writeTcpData(buf, len);
			if(wlen == 0) {
				return 0;
			}
			if(wlen < 0) {
				debug_w("SSL SHUTDOWN");
				/*
				 * If we received a close_notify and we
				 * still send something, then we have our
				 * own response close_notify to send, and
				 * the peer is allowed by RFC 5246 not to
				 * wait for it.
				 */
				if(!engine->shutdown_recv) {
					//				br_ssl_engine_fail(engine, BR_ERR_IO);
				}
				return BR_ERR_IO;
			}

			br_ssl_engine_sendrec_ack(engine, wlen);
			continue;
		}

		/*
		 * If we reached our target, then we are finished.
		 */
		if(state & target) {
			return state;
		}

		// Conflict: Application data hasn't been read
		if(state & BR_SSL_RECVAPP) {
			debug_e("SSL: Protocol Error");
			return BR_ERR_BAD_STATE;
		}

		if(state & BR_SSL_RECVREC) {
			size_t avail = 0;
			auto buf = br_ssl_engine_recvrec_buf(engine, &avail);
			auto len = input.read(buf, avail);
			if(len == 0) {
				return state;
			}

			debug_hex(DBG, "READ", buf, len);
			br_ssl_engine_recvrec_ack(engine, len);

			continue;
		}

		// Make room for new incoming records
		br_ssl_engine_flush(engine, 0);
	}
}

} // namespace Ssl
