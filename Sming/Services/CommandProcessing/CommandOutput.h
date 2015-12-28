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
#include "Print.h"
#include "WiringFrameworkDependencies.h"
#include "Network/WebSocket.h"

class CommandOutput: public Print
{
public:
	CommandOutput(TcpClient* reqClient);
	CommandOutput(Stream* reqStream);
	CommandOutput(WebSocket* reqSocket);
	virtual ~CommandOutput();

	size_t write(uint8_t outChar);

	TcpClient* outputTcpClient = nullptr;
	Stream*    outputStream = nullptr;
	WebSocket* outputSocket = nullptr;
	String tempSocket = "";
};

#endif /* SERVICES_COMMANDPROCESSING_COMMANDOUTPUT_H_ */
