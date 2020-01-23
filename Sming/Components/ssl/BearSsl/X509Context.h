/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * X509Context.h
 *
 ****/

#pragma once

#include <SslDebug.h>
#include "X509Name.h"
#include <Network/Ssl/Fingerprints.h>

namespace Ssl
{
/**
 * @brief Interface class to handle certificate processing
 */
class X509Handler
{
public:
	virtual void startChain(const char* serverName) = 0;
	virtual void startCert(uint32_t length) = 0;
	virtual void appendCertData(const uint8_t* buf, size_t len) = 0;
	virtual void endCert() = 0;
	virtual bool endChain() = 0;
	virtual const br_x509_pkey* getPublicKey() = 0;
};

/**
 * @brief C++ wrapper around a br_x509_class
 */
class X509Context
{
	// Require `this == &vtable`
	const br_x509_class* vtable = &x509_class;

public:
	X509Context(X509Handler& handler) : handler(handler)
	{
	}

	operator const br_x509_class**()
	{
		return &vtable;
	}

	unsigned count() const
	{
		return certificateCount;
	}

private:
#define GET_SELF() auto self = reinterpret_cast<X509Context*>(ctx)

	// Callback on the first byte of any certificate
	static void start_chain(const br_x509_class** ctx, const char* server_name)
	{
		debug_i("start_chain: %s", server_name);
		GET_SELF();
		self->certificateCount = 0;
		self->handler.startChain(server_name);
	}

	// Callback for each certificate present in the chain
	static void start_cert(const br_x509_class** ctx, uint32_t length)
	{
		debug_i("start_cert: %u", length);
		GET_SELF();
		self->startCert(length);
	}

	void startCert(uint32_t length)
	{
		handler.startCert(length);
	}

	// Callback for each byte stream in the chain
	static void append(const br_x509_class** ctx, const unsigned char* buf, size_t len)
	{
		debug_i("append: %u", len);
		GET_SELF();
		self->handler.appendCertData(buf, len);
		debug_hex(DBG, "CERT", buf, len, 0);
	}

	static void end_cert(const br_x509_class** ctx)
	{
		debug_i("end_cert");
		GET_SELF();
		self->handler.endCert();
		++self->certificateCount;
	}

	// Complete chain has been parsed, return 0 on validation success
	static unsigned end_chain(const br_x509_class** ctx)
	{
		debug_i("end_chain");
		GET_SELF();
		return self->endChain();
	}

	unsigned endChain()
	{
		if(certificateCount == 0) {
			debug_w("No certificate processed");
			return BR_ERR_X509_EMPTY_CHAIN;
		}

		if(!handler.endChain()) {
			return BR_ERR_X509_NOT_TRUSTED;
		}

		return BR_ERR_OK;
	}

#undef GET_SELF

	static const br_x509_pkey* get_pkey(const br_x509_class* const* ctx, unsigned* usages)
	{
		auto self = reinterpret_cast<const X509Context*>(ctx);
		if(usages != nullptr) {
			*usages = BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN;
		}
		return self->handler.getPublicKey();
	}

private:
	static const br_x509_class x509_class;
	X509Handler& handler;
	uint8_t certificateCount = 0;
};

static_assert(!std::is_polymorphic<X509Context>::value, "X509Context must not contain virtual methods");

} // namespace Ssl
