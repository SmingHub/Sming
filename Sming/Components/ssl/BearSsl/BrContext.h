/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrContext.h
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <Network/Ssl/Context.h>

namespace Ssl
{
/**
 * @brief Bear SSL uses separate contexts for clients and servers which are managed
 * by the Connection, not this class
 *
 */
class BrContext : public Context
{
public:
	using Context::Context;

	bool init() override
	{
		return true;
	}

	Connection* createClient(tcp_pcb* tcp) override;
	Connection* createServer(tcp_pcb* tcp) override;
};

} // namespace Ssl
