/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxtlsContext.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Ssl/SslInterface.h>
#include <axtls-8266/compat/lwipr_compat.h>

class SslContextImpl : public SslContext
{
public:
	bool init(tcp_pcb* tcp, uint32_t options = 0, size_t sessionCacheSize = 1) override;

	SslConnection* createServer() override;
	bool loadMemory(const SslContextObject& memType, const uint8_t* data, size_t length, const char* password) override;
	virtual ~SslContextImpl();

protected:
	SslConnection* doCreateClient(const uint8_t* sessionData, size_t length, SslExtension* sslExtensions) override;

private:
	uint8_t fdCount = 10;
	SSLCTX* context = nullptr;
};
