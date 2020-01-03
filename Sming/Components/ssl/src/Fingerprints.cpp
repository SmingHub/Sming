/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Fingerprints.cpp
 *
 ****/

#include <Network/Ssl/Fingerprints.h>
#include <user_config.h>

namespace Ssl
{
static void freeValue(const uint8_t*& ptr)
{
	delete[] ptr;
	ptr = nullptr;
}

void Fingerprints::free()
{
	freeValue(certSha1);
	freeValue(pkSha256);
}

bool Fingerprints::setValue(const uint8_t*& value, unsigned requiredLength, const uint8_t* newValue, unsigned newLength)
{
	if(newValue == nullptr || newLength == 0) {
		freeValue(value);
		return true;
	}

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

Fingerprints& Fingerprints::operator=(Fingerprints& source)
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

Fingerprints& Fingerprints::operator=(const Fingerprints& source)
{
	if(this != &source) {
		setSha1(source.certSha1, SHA1_SIZE);
		setSha256(source.pkSha256, SHA256_SIZE);
	}

	return *this;
}

} // namespace Ssl
