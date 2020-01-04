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
		dn.setLength(0);
	}

	const String& getDN() const
	{
		return dn;
	}

	static void append(void* ctx, const void* buf, size_t len)
	{
		auto self = reinterpret_cast<X509Name*>(ctx);
		self->dn.concat(static_cast<const char*>(buf), len);
	}

	// Obtain Relative Distinguished Name by type
	String getRDN(uint8_t type) const;

private:
	String dn; ///< The ASN1-encoded Distinguished Name
};

} // namespace Ssl
