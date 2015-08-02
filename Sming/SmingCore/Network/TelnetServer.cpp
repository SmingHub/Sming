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

void TelnetServer::setDebug(bool reqStatus)
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
		commandExecutor = new  CommandExecutor(client);
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
		delete commandExecutor;
		commandExecutor = nullptr;
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
	if (commandExecutor)
	{
		commandExecutor->executorReceive(data,size);
	}

	return true;
}
