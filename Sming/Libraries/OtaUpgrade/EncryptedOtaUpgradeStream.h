/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * EncryptedOtaUpgradeStream.h
 *
 ****/

#pragma once

#include "BasicOtaUpgradeStream.h"
#include <sodium/crypto_secretstream_xchacha20poly1305.h>

/**
 * @brief Encryption wrapper for #BasicOtaUpgradeStream.
 *
 * The class processes encrypted firmware upgrade files created by otatool.py.
 * A buffer is allocated dynamically to fit the largest chunk of the encryption container
 * (2kB unless otatool.py was modified). The actual processing of the decrypted data is 
 * defered to #BasicOtaUpgradeStream.
 */
class EncryptedOtaUpgradeStream : public BasicOtaUpgradeStream
{
public:
	EncryptedOtaUpgradeStream() = default;

	~EncryptedOtaUpgradeStream()
	{
		free(buffer);
	}

	/** @brief Process an arbitrarily sized chunk of an encrypted OTA upgrade file.
	 * @param data Pointer to chunk of data.
	 * @param size Size of chunk pointed to by \a data in bytes.
	 * @return If less than \a size, an error occured. Check \link BasicOtaUpgradeStream::errorCode \c errorCode \endlink for more details.
	 * @note \a size does not have to match the chunk size used by otatool.py
	 */
	size_t write(const uint8_t* data, size_t size) override;

private:
	crypto_secretstream_xchacha20poly1305_state state;

	union {
		uint8_t header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
		uint16_t chunkSizeMinusOne;
	};

	enum class Fragment {
		Header,
		ChunkSize,
		Chunk,
		None,
	};
	Fragment fragment = Fragment::Header;

	size_t remainingBytes = sizeof(header);
	uint8_t* fragmentPtr = header;
	uint8_t* buffer = nullptr;
	size_t bufferSize = 0;
};
