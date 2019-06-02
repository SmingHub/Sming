/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TelnetServer.h
 *
 *  Created on: 18 apr. 2015
 *      Author: Herman
 *
 ****/

/** @defgroup   telnetserver Telnet server
 *  @brief      Provides Telnet server
 *  @ingroup    tcpserver
 *  @{
 */

#pragma once

#include <user_config.h>
#include "Delegate.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "SystemClock.h"
#include "Services/CommandProcessing/CommandExecutor.h"

#define TELNETSERVER_MAX_COMMANDSIZE 64

typedef Delegate<void(TcpClient* client, char* data, int size)> TelnetServerCommandDelegate;

class TelnetServer : public TcpServer
{
public:
	TelnetServer()
	{
	}

	//	void setCommandDelegate(TelnetServerCommandDelegate reqDelegate);
	void enableDebug(bool reqStatus);
	void enableCommand(bool reqStatus);

private:
	void onClient(TcpClient* client) override;
	bool onClientReceive(TcpClient& client, char* data, int size) override;
	void onClientComplete(TcpClient& client, bool successful) override;

	void wrchar(char c);

private:
	TcpClient* curClient = nullptr;
	CommandExecutor* commandExecutor = nullptr;
	bool telnetDebug = true;
	bool telnetCommand = true;
};

/** @} */
