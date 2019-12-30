/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrContext.cpp
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "BrContext.h"
#include "BrClientConnection.h"
#include "BrServerConnection.h"
#include "BrError.h"

namespace Ssl
{
Connection* BrContext::createClient(tcp_pcb* tcp)
{
	auto connection = new BrClientConnection(*this, tcp);
	if(connection != nullptr) {
		int res = connection->init();
		if(res < 0) {
			debug_e("Connection init failed: %s", connection->getErrorString(res).c_str());
			delete connection;
			connection = nullptr;
		}
	}
	return connection;
}

Connection* BrContext::createServer(tcp_pcb* tcp)
{
	auto connection = new BrServerConnection(*this, tcp);
	if(connection != nullptr) {
		int res = connection->init();
		if(res < 0) {
			debug_e("Connection init failed: %s", connection->getErrorString(res).c_str());
			delete connection;
			connection = nullptr;
		}
	}
	return connection;
}

} // namespace Ssl
