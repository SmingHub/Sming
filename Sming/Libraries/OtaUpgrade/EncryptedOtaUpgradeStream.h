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

class EncryptedOtaUpgradeStream : public BasicOtaUpgradeStream
{
public:
	EncryptedOtaUpgradeStream();
	~EncryptedOtaUpgradeStream();

	size_t write(const uint8_t* data, size_t size) override;

private:
	crypto_secretstream_xchacha20poly1305_state state;

	union {
		uint8_t header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
		uint16_t chunkSizeMinusOne;
	};

	enum { FragmentHeader, FragmentChunkSize, FragmentChunk, FragmentNone } fragment = FragmentHeader;

	size_t remainingBytes;
	uint8_t* fragmentPtr;
	uint8_t* buffer = nullptr;
	size_t bufferSize = 0;
};
