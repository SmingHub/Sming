/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * X509Name.h
 *
 ****/

#pragma once

#include <bearssl.h>
#include <WString.h>

namespace Ssl
{
class X509Name
{
public:
	X509Name() = default;

	X509Name(const X509Name& name) : dn(name.dn)
	{
	}

	X509Name(X509Name&& name) : dn(std::move(name.dn))
	{
	}

	X509Name& operator=(X509Name&& name)
	{
		if(this != &name) {
			dn = std::move(name.dn);
		}
		return *this;
	}

	void clear()
	{
		br_sha256_init(&sha256);
		dn.setLength(0);
	}

	uint8_t* getHash(uint8_t hash[br_sha256_SIZE]) const
	{
		br_sha256_out(&sha256, hash);
		return hash;
	}

	static void append(void* ctx, const void* buf, size_t len)
	{
		reinterpret_cast<X509Name*>(ctx)->append(buf, len);
	}

	// Obtain Relative Distinguished Name by type
	String getRDN(uint8_t type) const;

private:
	void append(const void* buf, size_t len);

private:
	br_sha256_context sha256 = {};
	String dn; ///< The ASN1-encoded Distinguished Name
};

} // namespace Ssl
