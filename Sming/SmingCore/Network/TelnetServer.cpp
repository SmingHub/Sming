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
	curClient->sendString("Welcome to Sming / ESP6266 Telnet\r\n");
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
	debugf("Data : %s", data);

	for (int i=0;i<size;i++)
	{
		if (data[i] >= 240) // part of Telnet command
		{
			debugf("TelnetServer : Telnet Command ignored\r\n");
		}
		else
		{
			if (commandIndex < TELNETSERVER_MAX_COMMANDSIZE)
			{
				commandBuf[commandIndex++] = data[i];
				if ( data[i] == '\n')
				{
					commandBuf[commandIndex] = 0;
					processCommandLine (commandBuf, commandIndex -1);
					commandIndex = 0;
				}
			}
		}
	}
	curClient->send(prompt,1);
	return true;
}

void TelnetServer::processCommandLine( char *cmdBuf, int size)
{
	debugf("Telnet Command %s", cmdBuf);
	if (strncmp(cmdBuf,"help",4) == 0)
	{
		curClient->sendString("TelnetServer : Help\r\n");
		curClient->sendString("TelnetServer : Commands available\r\n");
		curClient->sendString("TelnetServer : close  -> Close current telnet session\r\n");
		curClient->sendString("TelnetServer : heap   -> Show free heap size\r\n");
		curClient->sendString("TelnetServer : status -> Show System status\r\n");
		curClient->sendString("TelnetServer : user   -> Call commandDelegate\r\n");
		curClient->sendString("TelnetServer : help   -> Show this information\r\n");
	}
	else
	if (strncmp(cmdBuf,"close",5) == 0)
	{
		curClient->close();
	}
	else
	if (strncmp(cmdBuf,"heap",4) == 0)
	{
		char tempBuf[100];
		int sz = sprintf(tempBuf,"Free heap size = %d\r\n", system_get_free_heap_size());
		curClient->send(tempBuf,sz);
	}
	else
	if (strncmp(cmdBuf,"status",6) == 0)
	{
		showStatus();
	}
	else
	if (strncmp(cmdBuf,"user",4) == 0)
	{
		if (commandDelegate)
		{
			commandDelegate(curClient, cmdBuf,size);
		}
		else
		{
			curClient->sendString("TelnetServer : No commandDelegate installed\r\n");
		}
	}
	else
	{
		curClient->sendString("TelnetServer : Unknown command\r\n");
	}
}

void TelnetServer::setCommandDelegate(TelnetServerCommandDelegate reqDelegate)
{
	commandDelegate = reqDelegate;
}

void TelnetServer::showStatus()
{
	char tempBuf[64];
	curClient->sendString("System information : ESP8266 Sming Framework\r\n");
	curClient->sendString("Sming Framework Version \r\n");
	curClient->sendString("ESP SDK version : ");
	curClient->sendString(system_get_sdk_version());
	curClient->sendString("\r\n");
	curClient->sendString("Time = ");
	curClient->sendString(SystemClock.getSystemTimeString());
	curClient->sendString("\r\n");
	int sz = sprintf(tempBuf,"System Start Reason : %d\r\n", system_get_rst_info()->reason);
	curClient->send(tempBuf,sz);
}
