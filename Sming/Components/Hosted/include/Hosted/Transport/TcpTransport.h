/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TcpTransport.h
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#pragma once

#include <Network/TcpServer.h>
#include "BaseTransport.h"

namespace Hosted::Transport
{
class TcpTransport : public BaseTransport
{
protected:
	virtual bool process(TcpClient& client, char* data, int size) = 0;
};

} // namespace Hosted::Transport
