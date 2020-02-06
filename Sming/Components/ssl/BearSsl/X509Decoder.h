/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * X509Decoder.h
 *
 ****/

#pragma once

#include "X509Name.h"

namespace Ssl
{
/**
 * @brief Wrapper for a br_x509_decoder_context
 */
class X509Decoder
{
public:
	X509Decoder(X509Name* subject = nullptr, X509Name* issuer = nullptr)
	{
		br_x509_decoder_init(&context, subject ? X509Name::append : nullptr, subject,
							 issuer ? X509Name::append : nullptr, issuer);
	}

	void push(const uint8_t* buf, size_t len)
	{
		br_x509_decoder_push(&context, buf, len);
	}

	br_x509_pkey* getPublicKey()
	{
		return br_x509_decoder_get_pkey(&context);
	}

private:
	br_x509_decoder_context context;
};

} // namespace Ssl
