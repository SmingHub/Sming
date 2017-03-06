/*
 * telnetServer.cpp
 *
 *  Created on: 18 apr. 2015
 *      Author: Herman
 */

#include "TelnetServer.h"
#include "TcpServer.h"

TelnetServer::TelnetServer() : TcpServer()
{
	// TODO Auto-generated constructor stub

}

TelnetServer::~TelnetServer()
{
	// TODO Auto-generated destructor stub
}

void TelnetServer::enableDebug(bool reqStatus)
{
	telnetDebug = reqStatus;
	if (telnetDebug && curClient) /* only setSetDebug when already connected */
	{
		Debug.setDebug(DebugPrintCharDelegate(&TelnetServer::wrchar,this));
	}
	else
	{
		Debug.setDebug(Serial);
	}
}

void TelnetServer::enableCommand(bool reqStatus)
{
#if !DISABLE_CMD_EXEC
	if (reqStatus && curClient && !commandExecutor)
	{
		commandExecutor = new  CommandExecutor(curClient);
	}
	if (!reqStatus && commandExecutor)
	{
		delete commandExecutor;
		commandExecutor = nullptr;
	}
#endif
	telnetCommand = reqStatus;
}
void TelnetServer::onClient(TcpClient *client)
{
	debugf("TelnetServer onClient %s", client->getRemoteIp().toString().c_str() );

	TcpServer::onClient(client);

	if (curClient)
	{
		debugf("TCP Client already connected");
		client->sendString("Telnet Client already connected\r\n");
		client->close();
	}
	else
	{
		curClient = client;
		curClient->setTimeOut(USHRT_MAX);
		curClient->sendString("Welcome to Sming / ESP6266 Telnet\r\n");
		if (telnetCommand)
		{
#if !DISABLE_CMD_EXEC
			commandExecutor = new  CommandExecutor(client);
#endif
		}
		if (telnetDebug)
		{
			Debug.setDebug(DebugPrintCharDelegate(&TelnetServer::wrchar,this));
		}
		Debug.printf("This is debug after telnet start\r\n");
	}
}

void TelnetServer::onClientComplete(TcpClient& client, bool succesfull)
{
	if ( &client == curClient)
	{
#if !DISABLE_CMD_EXEC
		delete commandExecutor;
		commandExecutor = nullptr;
#endif
		curClient = nullptr;
		debugf("TelnetServer onClientComplete %s", client.getRemoteIp().toString().c_str() );
	}
	else
	{
		debugf("Telnet server unconnected client close");
	}

	debugf("TelnetServer onClientComplete %s", client.getRemoteIp().toString().c_str() );
	TcpServer::onClientComplete(client, succesfull);
	Debug.setDebug(Serial);
}

void TelnetServer::wrchar(char c)
{
	char ca[2];
	ca[0]= c;
	curClient->write(ca,1);
}

bool TelnetServer::onClientReceive (TcpClient& client, char *data, int size)
{
	debugf("TelnetServer onClientReceive : %s, %d bytes \r\n",client.getRemoteIp().toString().c_str(),size );
	debugf("Data : %s", data);
#if !DISABLE_CMD_EXEC
	if (commandExecutor)
	{
		commandExecutor->executorReceive(data,size);
	}
#endif
	return true;
}
