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

#include "TcpClient.h"
#include "TcpServer.h"
#include <SystemClock.h>
#include <Services/CommandProcessing/Handler.h>

#ifndef TELNETSERVER_MAX_COMMANDSIZE
#define TELNETSERVER_MAX_COMMANDSIZE 64
#endif

using TelnetServerCommandDelegate = Delegate<void(TcpClient* client, char* data, int size)>;

class TelnetServer : public TcpServer
{
public:
	void enableDebug(bool reqStatus) SMING_DEPRECATED
	{
	}

	void enableCommand(bool reqStatus);

private:
	void onClient(TcpClient* client) override;
	bool onClientReceive(TcpClient& client, char* data, int size) override;
	void onClientComplete(TcpClient& client, bool successful) override;

	void wrchar(char c);

private:
	TcpClient* curClient = nullptr;
	CommandProcessing::Handler* commandHandler = nullptr;
	bool telnetCommand = true;
};

/** @} */
