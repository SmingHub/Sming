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

void TelnetServer::onClient(TcpClient *client)
{
	debugf("TelnetServer onClient %s", client->getRemoteIp().toString().c_str() );
	curClient = client;
	TcpServer::onClient(client);
}

void TelnetServer::onClientComplete(TcpClient& client, bool succesfull)
{
	curClient = nullptr;
	debugf("TelnetServer onClientComplete %s", client.getRemoteIp().toString().c_str() );
	TcpServer::onClientComplete(client, succesfull);
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
//	debugf("TelnetServer callback : %d, %d bytes \r\n",client.getRemoteIp()[0],size );

	debugf("Data : %s", data);
	if (strcmp(data,"close") == 0)
	{
		debugf("Closing client");
		client.close();
	};
	if (data[0] == 255)
	{
      debugf("TelnetServer : Telnet Command received");
	}
	return true;
}
