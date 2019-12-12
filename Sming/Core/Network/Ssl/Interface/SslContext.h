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

#pragma once

#include <user_config.h>
#include "../SslStructs.h"
#include "SslExtension.h"

class SslConnection;

class SslContext
{
public:
	virtual bool init(tcp_pcb* tcp, uint32_t options = 0, size_t sessionCacheSize = 1)
	{
		this->tcp = tcp;
		this->options = options;
		this->sessionCacheSize = sessionCacheSize;
		return true;
	}

	SslConnection* createClient(SslSessionId* sessionId, SslExtension* sslExtensions)
	{
		return doCreateClient(
					sessionId != nullptr ? sessionId->getValue() : nullptr,
					sessionId != nullptr ? sessionId->getLength() : 0,
				    sslExtensions);
	}

	virtual SslConnection* createServer() = 0;
	virtual bool loadMemory(int memType, const uint8_t *data, size_t length,
			const char *password) = 0;

	bool isValid() {
		return valid;
	}

	tcp_pcb& getTcp() {
		return *tcp;
	}

	virtual ~SslContext()
	{}

protected:
	virtual SslConnection* doCreateClient(const uint8_t* sessionData, size_t length, SslExtension* sslExtensions) = 0;

protected:
	tcp_pcb* tcp = nullptr;
	uint32_t options;
	size_t sessionCacheSize;
	bool valid = false;
};
