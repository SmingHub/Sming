/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * EncryptedOtaUpgradeStream.cpp
 *
 ****/

#include "EncryptedOtaUpgradeStream.h"
#include <sodium/utils.h>
#include <FlashString/Array.hpp>

DECLARE_FSTR_ARRAY(OTAUpgrade_DecryptionKey, uint8_t)

EncryptedOtaUpgradeStream::EncryptedOtaUpgradeStream() : BasicOtaUpgradeStream()
{
	fragmentPtr = header;
	remainingBytes = sizeof(header);
}

EncryptedOtaUpgradeStream::~EncryptedOtaUpgradeStream()
{
	free(buffer);
}

size_t EncryptedOtaUpgradeStream::write(const uint8_t* data, size_t size)
{
	const size_t origSize = size;

	while(!hasError() && (size > 0)) {
		std::size_t toConsume = std::min(remainingBytes, size);
		memcpy(fragmentPtr, data, toConsume);
		size -= toConsume;
		data += toConsume;
		fragmentPtr += toConsume;
		remainingBytes -= toConsume;

		if(remainingBytes == 0) {
			switch(fragment) {
			case FragmentHeader: {
				assert(OTAUpgrade_DecryptionKey.length() == crypto_secretstream_xchacha20poly1305_KEYBYTES);
				LOAD_FSTR_ARRAY(key, OTAUpgrade_DecryptionKey);
				bool ok = (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) == 0);
				sodium_memzero(key, sizeof(key));
				if(!ok) {
					setError(DecryptionError);
					break;
				}
				fragment = FragmentChunkSize;
				fragmentPtr = reinterpret_cast<uint8_t*>(&chunkSizeMinusOne);
				remainingBytes = sizeof(chunkSizeMinusOne);
			} break;

			case FragmentChunkSize:
				remainingBytes = 1 + chunkSizeMinusOne;
				if(buffer == nullptr || bufferSize < remainingBytes) {
					free(buffer);
					buffer = (uint8_t*)malloc(remainingBytes);
					if(buffer == nullptr) {
						setError(OutOfMemoryError);
						break;
					}
					bufferSize = remainingBytes;
				}
				fragmentPtr = buffer;
				fragment = FragmentChunk;
				break;

			case FragmentChunk: {
				unsigned char tag;
				size_t chiperTextLength = 1 + chunkSizeMinusOne;
				unsigned long long messageLength = 0;
				bool ok = (crypto_secretstream_xchacha20poly1305_pull(&state, buffer, &messageLength, &tag, buffer,
																	  chiperTextLength, NULL, 0) == 0);
				if(!ok || messageLength > bufferSize) {
					setError(DecryptionError);
					break;
				}
				if(tag != crypto_secretstream_xchacha20poly1305_TAG_FINAL) {
					fragment = FragmentChunkSize;
					fragmentPtr = reinterpret_cast<uint8_t*>(&chunkSizeMinusOne);
					remainingBytes = sizeof(chunkSizeMinusOne);
				} else {
					fragment = FragmentNone;
				}

				BasicOtaUpgradeStream::write(buffer, static_cast<size_t>(messageLength));
			} break;

			case FragmentNone:
				setError(InvalidFormatError);
				break;

			default:
				setError(InternalError);
			}
		}
	}
	return origSize - size;
}
