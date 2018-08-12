/*
 * telnetServer.h
 *
 *  Created on: 18 apr. 2015
 *      Author: Herman
 */

/** @defgroup   telnetserver Telnet server
 *  @brief      Provides Telnet server
 *  @ingroup    tcpserver
 *  @{
 */

#ifndef APP_TELNETSERVER_H_
#define APP_TELNETSERVER_H_

#include <user_config.h>
#include "../Delegate.h"
#include "../Debug.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "SystemClock.h"
#include "../Services/CommandProcessing/CommandExecutor.h"

#include <stdio.h>

#define TELNETSERVER_MAX_COMMANDSIZE 64

typedef Delegate<void(TcpClient* client, char* data, int size)> TelnetServerCommandDelegate;

class TelnetServer : public TcpServer {
public:
	TelnetServer();
	virtual ~TelnetServer();
	//	void setCommandDelegate(TelnetServerCommandDelegate reqDelegate);
	void enableDebug(bool reqStatus);
	void enableCommand(bool reqStatus);

private:
	void onClient(TcpClient* client);
	bool onClientReceive(TcpClient& client, char* data, int size);
	void onClientComplete(TcpClient& client, bool succesfull);
	void wrchar(char c);
	TcpClient* curClient = nullptr;
	CommandExecutor* commandExecutor = nullptr;
	bool telnetDebug = true;
	bool telnetCommand = true;
};

/** @} */
#endif /* APP_TELNETSERVER_H_ */
