/*
 * CommandOutput.cpp
 *
 *  Created on: 5 jul. 2015
 *      Author: Herman
 */

#include "CommandOutput.h"

size_t CommandOutput::write(uint8_t outChar)
{
	if (_outputTcpClient)
		return _outputTcpClient->write((const char*)&outChar, 1);

	if (_outputStream)
		return _outputStream->write(outChar);

	if (_outputSocket) {
		if (outChar == '\r') {
			_outputSocket->sendString(_tempSocket);
			_tempSocket.setLength(0);
		}
		else
			_tempSocket += char(outChar);

		return 1;
	}

	return 0;
}
