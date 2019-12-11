/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslInterface.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "AxtlsCertificate.h"

const String AxtlsCertificate::getName(const String& name) const
{
  if(ssl == nullptr) {
	  return String();
  }

  return String(ssl_get_cert_dn(ssl, SSL_X509_CERT_COMMON_NAME));
}

bool AxtlsCertificate::matchFingerprint(const uint8_t* hash) const
{
	return (ssl_match_fingerprint(ssl, hash) == 0);
}

bool AxtlsCertificate::matchPki(const uint8_t* hash) const
{
	return (ssl_match_spki_sha256(ssl, hash) == 0);
}
