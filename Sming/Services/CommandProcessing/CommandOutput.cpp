/*
 * CommandOutput.cpp
 *
 *  Created on: 5 jul. 2015
 *      Author: Herman
 */

#include "CommandOutput.h"
#include <debug_progmem.h>

CommandOutput::CommandOutput(Stream* reqStream) : outputStream(reqStream)
{
}

CommandOutput::~CommandOutput()
{
	debugf("destruct");
}

size_t CommandOutput::write(uint8_t outChar)
{
	if(outputStream) {
		return outputStream->write(outChar);
	}

#ifndef DISABLE_NETWORK
	if(outputTcpClient) {
		char outBuf[1] = {char(outChar)};
		return outputTcpClient->write(outBuf, 1);
	}
	if(outputSocket) {
		if(outChar == '\r') {
			outputSocket->sendString(tempSocket);
			tempSocket = "";
		} else {
			tempSocket = tempSocket + String(char(outChar));
		}

		return 1;
	}
#endif

	return 0;
}
