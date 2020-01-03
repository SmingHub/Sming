/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * KeyCertPair.cpp
 *
 ****/

#include <Network/Ssl/KeyCertPair.h>

namespace Ssl
{
bool KeyCertPair::assign(const uint8_t* newKey, unsigned newKeyLength, const uint8_t* newCertificate,
						 unsigned newCertificateLength, const char* newKeyPassword)
{
	free();

	if(newKeyLength != 0 && newKey != nullptr) {
		if(!key.setLength(newKeyLength)) {
			return false;
		}
		memcpy(key.begin(), newKey, newKeyLength);
	}

	if(newCertificateLength != 0 && newCertificate != nullptr) {
		if(!certificate.setLength(newCertificateLength)) {
			return false;
		}
		memcpy(certificate.begin(), newCertificate, newCertificateLength);
	}

	return setPassword(newKeyPassword);
}

bool KeyCertPair::setPassword(const char* newKeyPassword)
{
	unsigned passwordLength = (newKeyPassword == nullptr) ? 0 : strlen(newKeyPassword);
	if(passwordLength == 0) {
		keyPassword = nullptr;
		return true;
	}

	keyPassword.setString(newKeyPassword, passwordLength);
	return keyPassword;
}

} // namespace Ssl
