/*
 * telnetServer.cpp
 *
 *  Created on: 18 apr. 2015
 *      Author: Herman
 */

#include "TelnetServer.h"
#include "Debug.h"
#include "HardwareSerial.h"

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
	_telnetDebug = reqStatus;
	// only setSetDebug when already connected
	if (_telnetDebug && _curClient)
		Debug.setDebug(DebugPrintCharDelegate(&TelnetServer::wrchar, this));
	else
		Debug.setDebug(Serial);
}

void TelnetServer::enableCommand(bool reqStatus)
{
#if ENABLE_CMD_EXECUTOR
	if (reqStatus && _curClient && !_commandExecutor)
		_commandExecutor = new CommandExecutor(_curClient);

	if (!reqStatus && _commandExecutor) {
		delete _commandExecutor;
		_commandExecutor = nullptr;
	}
#endif
	_telnetCommand = reqStatus;
}
void TelnetServer::onClient(TcpClient* client)
{
	debug_d("TelnetServer onClient %s", client->getRemoteIp().toString().c_str());

	TcpServer::onClient(client);

	if (_curClient) {
		debug_d("TCP Client already connected");
		client->sendString("Telnet Client already connected\r\n");
		client->close();
	}
	else {
		_curClient = client;
		_curClient->setTimeOut(USHRT_MAX);
		_curClient->sendString("Welcome to Sming / ESP6266 Telnet\r\n");
		if (_telnetCommand)
#if ENABLE_CMD_EXECUTOR
			delete _commandExecutor;
			_commandExecutor = new CommandExecutor(client);
#endif

		if (_telnetDebug)
			Debug.setDebug(DebugPrintCharDelegate(&TelnetServer::wrchar, this));

		Debug.printf("This is debug after telnet start\r\n");
	}
}

void TelnetServer::onClientComplete(TcpClient& client, bool succesfull)
{
	if (&client == _curClient) {
#if ENABLE_CMD_EXECUTOR
		delete _commandExecutor;
		_commandExecutor = nullptr;
#endif
		_curClient = nullptr;
		debug_d("TelnetServer onClientComplete %s", client.getRemoteIp().toString().c_str());
	}
	else {
		debug_d("Telnet server unconnected client close");
	}

	debug_d("TelnetServer onClientComplete %s", client.getRemoteIp().toString().c_str());
	TcpServer::onClientComplete(client, succesfull);
	Debug.setDebug(Serial);
}

void TelnetServer::wrchar(char c)
{
	char ca[2];
	ca[0] = c;
	_curClient->write(ca, 1);
}

bool TelnetServer::onClientReceive(TcpClient& client, char* data, int size)
{
	debug_d("TelnetServer onClientReceive : %s, %d bytes \r\n", client.getRemoteIp().toString().c_str(), size);
	debug_d("Data : %s", data);
#if ENABLE_CMD_EXECUTOR
	if (_commandExecutor)
		_commandExecutor->executorReceive(data, size);
#endif
	return true;
}
