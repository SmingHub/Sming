/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * EncryptedOtaUpgradeStream.cpp
 *
 ****/

#include "EncryptedStream.h"
#include <sodium/utils.h>
#include <FlashString/Array.hpp>

namespace OtaUpgrade
{
DECLARE_FSTR_ARRAY(DecryptionKey, uint8_t)

size_t EncryptedStream::write(const uint8_t* data, size_t size)
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
			case Fragment::Header: {
				assert(DecryptionKey.length() == crypto_secretstream_xchacha20poly1305_KEYBYTES);
				LOAD_FSTR_ARRAY(key, DecryptionKey);
				bool ok = (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) == 0);
				sodium_memzero(key, sizeof(key));
				if(!ok) {
					setError(Error::DecryptionFailed);
					break;
				}
				fragment = Fragment::ChunkSize;
				fragmentPtr = reinterpret_cast<uint8_t*>(&chunkSizeMinusOne);
				remainingBytes = sizeof(chunkSizeMinusOne);
			} break;

			case Fragment::ChunkSize:
				remainingBytes = 1 + chunkSizeMinusOne;
				if(!buffer || bufferSize < remainingBytes) {
					buffer.reset(new uint8_t[remainingBytes]);
					if(!buffer) {
						setError(Error::OutOfMemory);
						break;
					}
					bufferSize = remainingBytes;
				}
				fragmentPtr = buffer.get();
				fragment = Fragment::Chunk;
				break;

			case Fragment::Chunk: {
				unsigned char tag;
				size_t chiperTextLength = 1 + chunkSizeMinusOne;
				unsigned long long messageLength = 0;
				bool ok = (crypto_secretstream_xchacha20poly1305_pull(&state, buffer.get(), &messageLength, &tag,
																	  buffer.get(), chiperTextLength, nullptr, 0) == 0);
				if(!ok || messageLength > bufferSize) {
					setError(Error::DecryptionFailed);
					break;
				}
				if(tag != crypto_secretstream_xchacha20poly1305_TAG_FINAL) {
					fragment = Fragment::ChunkSize;
					fragmentPtr = reinterpret_cast<uint8_t*>(&chunkSizeMinusOne);
					remainingBytes = sizeof(chunkSizeMinusOne);
				} else {
					fragment = Fragment::None;
				}

				BasicStream::write(buffer.get(), size_t(messageLength));
			} break;

			case Fragment::None:
				setError(Error::InvalidFormat);
				break;

			default:
				setError(Error::Internal);
			}
		}
	}
	return origSize - size;
}

} // namespace OtaUpgrade
