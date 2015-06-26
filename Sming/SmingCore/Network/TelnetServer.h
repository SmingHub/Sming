/*
 * telnetServer.h
 *
 *  Created on: 18 apr. 2015
 *      Author: Herman
 */

#ifndef APP_TELNETSERVER_H_
#define APP_TELNETSERVER_H_

#include <user_config.h>
#include "../Delegate.h"
// #include "../Debug.h"
#include "TcpClient.h"
#include "TcpServer.h"


class TelnetServer : public TcpServer
{
public:
	TelnetServer();
	virtual ~TelnetServer();

private:
	void onClient(TcpClient *client);
	bool onClientReceive (TcpClient& client, char *data, int size);
	void onClientComplete(TcpClient& client, bool succesfull);
	void wrchar(char c);
	TcpClient *curClient;

};

#endif /* APP_TELNETSERVER_H_ */
