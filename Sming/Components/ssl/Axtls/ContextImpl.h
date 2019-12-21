/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ContextImpl.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Ssl/Context.h>
#include <axtls-8266/compat/lwipr_compat.h>

namespace Ssl
{
class ContextImpl : public Context
{
public:
	~ContextImpl();

	bool init(tcp_pcb* tcp, uint32_t options, size_t sessionCacheSize) override;
	Connection* createServer() override;
	bool loadMemory(ObjectType memType, const uint8_t* data, size_t length, const char* password) override;

protected:
	Connection* internalCreateClient(const uint8_t* sessionData, size_t length, Extension* extension) override;

private:
	uint8_t capacity = 10;
	SSLCTX* context = nullptr;
};

} // namespace Ssl
