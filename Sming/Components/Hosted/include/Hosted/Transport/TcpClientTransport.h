/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TcpClientTransport.h
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#pragma once

#include <Network/TcpClient.h>
#include "TcpTransport.h"
#include "TcpClientStream.h"

namespace Hosted
{
namespace Transport
{
class TcpClientTransport : public TcpTransport
{
public:
	TcpClientTransport(TcpClient& client)
	{
		client.setReceiveDelegate(TcpClientDataDelegate(&TcpClientTransport::process, this));
		stream = new TcpClientStream(client);
	}

	~TcpClientTransport()
	{
		delete stream;
	}

protected:
	bool process(TcpClient& client, char* data, int size) override
	{
		if(!stream.push(data, size)) {
			return false;
		}

		return handler(*stream);
	}

private:
	TcpClientStream* stream = nullptr;
};

} // namespace Transport

} // namespace Hosted
