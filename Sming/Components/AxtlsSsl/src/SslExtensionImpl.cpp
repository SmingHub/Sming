/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxtlsExtension.h
 *
 * SSL Extension implementation based on axTLS library
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "SslExtensionImpl.h"

SslExtensionImpl::SslExtensionImpl()
{
	sslExtension = ssl_ext_new();
}

bool SslExtensionImpl::setHostName(const String& hostName)
{
	if(sslExtension == nullptr) {
		return false;
	}

	ssl_ext_set_host_name(sslExtension, hostName.c_str());

	return true;
}

bool SslExtensionImpl::setMaxFragmentSize(const SslExtensionFragmentSize& fragmentSize)
{
	if(sslExtension == nullptr) {
		return false;
	}

	ssl_ext_set_max_fragment_size(sslExtension, fragmentSize);

	return true;
}
