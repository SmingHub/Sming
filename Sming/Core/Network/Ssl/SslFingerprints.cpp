/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslFingerprints.cpp
 *
 ****/

#ifdef ENABLE_SSL

#include "SslFingerprints.h"
#include <user_config.h>

static inline void freeValue(const uint8_t*& ptr)
{
	delete[] ptr;
	ptr = nullptr;
}

void SslFingerprints::free()
{
	freeValue(certSha1);
	freeValue(pkSha256);
}

bool SslFingerprints::setValue(const uint8_t*& value, unsigned requiredLength, const uint8_t* newValue,
							   unsigned newLength)
{
	if(newValue == nullptr || newLength == 0) {
		freeValue(value);
		return true;
	} else {
		if(newLength != requiredLength) {
			debug_w("Warning: Invalid fingerprint length");
			// Copy data anyway to prevent false positive validation
		}
		if(value == nullptr) {
			value = new uint8_t[requiredLength];
			if(value == nullptr) {
				return false;
			}
		}
		// If new value is longer than buffer, copy short
		unsigned length = std::min(newLength, requiredLength);
		// Behave properly when source is flash memory and length is wrong or buffers misaligned
		if(isFlashPtr(newValue)) {
			memcpy_P(const_cast<uint8_t*>(value), newValue, length);
		} else {
			memcpy(const_cast<uint8_t*>(value), newValue, length);
		}
		return true;
	}
}

SslFingerprints& SslFingerprints::operator=(SslFingerprints& source)
{
	if(this != &source) {
		freeValue(certSha1);
		certSha1 = source.certSha1;
		source.certSha1 = nullptr;

		freeValue(pkSha256);
		pkSha256 = source.pkSha256;
		source.pkSha256 = nullptr;
	}

	return *this;
}

/** @brief Make copy of values from source */
SslFingerprints& SslFingerprints::operator=(const SslFingerprints& source)
{
	if(this != &source) {
		setSha1(source.certSha1, SHA1_SIZE);
		setSha256(source.pkSha256, SHA256_SIZE);
	}

	return *this;
}

#endif // ENABLE_SSL
