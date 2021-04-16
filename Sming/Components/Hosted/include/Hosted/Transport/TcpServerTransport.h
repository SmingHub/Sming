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

namespace Hosted
{
namespace Transport
{
class TcpServerTransport : public TcpTransport
{
public:
	using ClientMap = ObjectMap<uint32_t, TcpClientStream>;

	TcpServerTransport(TcpServer& server)
	{
		server.setClientReceiveHandler(TcpClientDataDelegate(&TcpServerTransport::process, this));
	}

protected:
	bool process(TcpClient& client, char* data, int size) override
	{
		uint32_t key = uint32_t(&client);

		TcpClientStream* stream = nullptr;

		int i = map.indexOf(key);
		if(i >= 0) {
			stream = map.valueAt(i);
		} else {
			map[key] = new TcpClientStream(client);
			client.setReceiveDelegate(TcpClientDataDelegate(&TcpServerTransport::process, this));
			stream = map[key];
		}

		if(!stream->push(reinterpret_cast<const uint8_t*>(data), size)) {
			return false;
		}

		return handler(*stream);
	}

private:
	ClientMap map;
};

} // namespace Transport

} // namespace Hosted
