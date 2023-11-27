/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TelnetServer.cpp
 *
 *  Created on: 18 apr. 2015
 *      Author: Herman
 *
 ****/

#include "TelnetServer.h"
#include "TcpClientStream.h"

void TelnetServer::enableCommand(bool reqStatus)
{
	if(!reqStatus && commandHandler != nullptr) {
		delete commandHandler;
		commandHandler = nullptr;
		return;
	}

	if(reqStatus && curClient != nullptr && commandHandler == nullptr) {
		commandHandler = new CommandProcessing::Handler();
		commandHandler->setOutputStream(new TcpClientStream(*curClient));
	}

	telnetCommand = reqStatus;
}

void TelnetServer::onClient(TcpClient* client)
{
	debug_d("TelnetServer onClient %s", client->getRemoteIp().toString().c_str());

	TcpServer::onClient(client);

	if(curClient != nullptr) {
		debug_d("TCP Client already connected");
		client->sendString("Telnet Client already connected\r\n");
		client->close();
	} else {
		curClient = client;
		curClient->setTimeOut(USHRT_MAX);
		curClient->sendString("Welcome to Sming Telnet\r\n");
		if(telnetCommand) {
			commandHandler = new CommandProcessing::Handler();
			commandHandler->setOutputStream(new TcpClientStream(*client));
		}
	}
}

void TelnetServer::onClientComplete(TcpClient& client, bool successful)
{
	if(&client == curClient) {
		delete commandHandler;
		commandHandler = nullptr;
		curClient = nullptr;
		debug_d("TelnetServer onClientComplete %s", client.getRemoteIp().toString().c_str());
	} else {
		debug_d("Telnet server unconnected client close");
	}

	debug_d("TelnetServer onClientComplete %s", client.getRemoteIp().toString().c_str());
	TcpServer::onClientComplete(client, successful);
}

void TelnetServer::wrchar(char c)
{
	char ca[2];
	ca[0] = c;
	curClient->write(ca, 1);
}

bool TelnetServer::onClientReceive(TcpClient& client, char* data, int size)
{
	debug_d("TelnetServer onClientReceive : %s, %d bytes \r\n", client.getRemoteIp().toString().c_str(), size);
	debug_d("Data : %s", data);
	if(commandHandler != nullptr) {
		commandHandler->process(data, size);
	}
	return true;
}
