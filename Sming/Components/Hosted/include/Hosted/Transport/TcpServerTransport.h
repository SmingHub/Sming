/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TcpServerTransport.h
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#pragma once

#include <Network/TcpServer.h>
#include <Data/ObjectMap.h>
#include "TcpTransport.h"
#include "TcpClientStream.h"

namespace Hosted::Transport
{
class TcpServerTransport : public TcpTransport
{
public:
	TcpServerTransport(TcpServer& server)
	{
		server.setClientReceiveHandler(TcpClientDataDelegate(&TcpServerTransport::process, this));
	}

protected:
	bool process(TcpClient& client, char* data, int size) override
	{
		auto key = &client;

		TcpClientStream* stream = map.find(key);
		if(stream == nullptr) {
			map[key] = stream = new TcpClientStream(client);
			client.setReceiveDelegate(TcpClientDataDelegate(&TcpServerTransport::process, this));
		}

		if(!stream->push(reinterpret_cast<const uint8_t*>(data), size)) {
			return false;
		}

		return handler(*stream);
	}

private:
	using ClientMap = ObjectMap<TcpClient*, TcpClientStream>;

	ClientMap map;
};

} // namespace Hosted::Transport
