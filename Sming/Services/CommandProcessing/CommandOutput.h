/*
 * CommandOutput.h
 *
 *  Created on: 5 jul. 2015
 *      Author: Herman
 */

#ifndef SERVICES_COMMANDPROCESSING_COMMANDOUTPUT_H_
#define SERVICES_COMMANDPROCESSING_COMMANDOUTPUT_H_

#include "Network/TcpClient.h"
#include "Stream.h"
#include "WiringFrameworkDependencies.h"
#include "Network/Http/Websocket/WebSocketConnection.h"

class CommandOutput : public Print {
public:
	CommandOutput(TcpClient* reqClient) : _outputTcpClient(reqClient)
	{}

	CommandOutput(Stream* reqStream) : _outputStream(reqStream)
	{}

	CommandOutput(WebSocketConnection* reqSocket) : _outputSocket(reqSocket)
	{}

	virtual ~CommandOutput()
	{
		debugf("destruct");
	}

	size_t write(uint8_t outChar);

private:
	TcpClient* _outputTcpClient = nullptr;
	Stream* _outputStream = nullptr;
	WebSocketConnection* _outputSocket = nullptr;
	String _tempSocket = nullptr;
};

#endif /* SERVICES_COMMANDPROCESSING_COMMANDOUTPUT_H_ */
