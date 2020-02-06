/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxContext.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Ssl/Context.h>
#include <axtls-8266/ssl/ssl.h>

namespace Ssl
{
class AxContext : public Context
{
public:
	using Context::Context;
	~AxContext();

	bool init() override;
	Connection* createClient(tcp_pcb* pcb) override;
	Connection* createServer(tcp_pcb* pcb) override;

private:
	SSL_CTX* context = nullptr;
	int lastError = SSL_OK;
};

} // namespace Ssl
