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

namespace Ssl
{
#define GET_SELF() auto self = reinterpret_cast<X509Context*>(ctx)

class X509Context
{
public:
	using OnValidate = Delegate<bool()>;

	X509Context(OnValidate onValidate) : onValidate(onValidate)
	{
	}

	operator const br_x509_class**()
	{
		return &vtable;
	}

	const X509Name& getIssuer() const
	{
		return issuer;
	}

	const X509Name& getSubject() const
	{
		return subject;
	}

	bool matchFingerprint(const uint8_t* sha1Hash) const
	{
		uint8_t certHash[br_sha1_SIZE];
		br_sha1_out(&certificateSha1, certHash);
		return memcmp(certHash, sha1Hash, br_sha1_SIZE) == 0;
	}

	bool matchPki(const uint8_t* hash) const
	{
		// @todo
		return false;
	}

private:
	// Callback on the first byte of any certificate
	static void start_chain(const br_x509_class** ctx, const char* server_name)
	{
		debug_d("start_chain: %s", server_name);
		GET_SELF();
		self->startChain(server_name);
	}

	void startChain(const char* serverName);

	// Callback for each certificate present in the chain
	static void start_cert(const br_x509_class** ctx, uint32_t length)
	{
		debug_d("start_cert: %u", length);
		(void)ctx;
		(void)length;
	}

	// Callback for each byte stream in the chain
	static void append(const br_x509_class** ctx, const unsigned char* buf, size_t len)
	{
		debug_d("append: %u", len);
		GET_SELF();
		// Don't process anything but the first certificate in the chain
		if(self->certificateCount == 0) {
			br_sha1_update(&self->certificateSha1, buf, len);
			br_x509_decoder_push(&self->x509Decoder, (const void*)buf, len);
			debug_hex(DBG, "CERT", buf, len);
		}
	}

	static void end_cert(const br_x509_class** ctx)
	{
		debug_d("end_cert");
		GET_SELF();
		++self->certificateCount;
	}

	// Complete chain has been parsed, return 0 on validation success
	static unsigned end_chain(const br_x509_class** ctx)
	{
		debug_d("end_chain");
		GET_SELF();
		return self->endChain();
	}

	unsigned endChain();

	// Return the public key from the validator (set by x509_minimal)
	static const br_x509_pkey* get_pkey(const br_x509_class* const* ctx, unsigned* usages)
	{
		auto self = reinterpret_cast<const X509Context*>(ctx);
		if(usages != nullptr) {
			*usages = BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN;
		}
		return &self->x509Decoder.pkey;
	}

private:
	const br_x509_class* vtable = &vt;
	static const br_x509_class vt;
	OnValidate onValidate;
	br_sha1_context certificateSha1 = {};
	X509Name issuer;
	X509Name subject;
	br_x509_decoder_context x509Decoder = {};
	bool allowSelfSigned = false;
	uint8_t certificateCount = 0;
};

} // namespace Ssl
