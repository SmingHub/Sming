/*
 * ArduCamCommand.h
 *
 */

#pragma once

#include "WString.h"
#include <CommandProcessing/Handler.h>
#include <Libraries/ArduCAM/ArduCAM.h>

class ArduCamCommand
{
public:
	ArduCamCommand(ArduCAM& CAM, CommandProcessing::Handler& commandHandler);
	void initCommand();
	const char* getContentType();
	void setSize(const String& size);
	void setType(const String& type);

private:
	ArduCAM myCAM;
	CommandProcessing::Handler& commandHandler;
	uint8_t imgType;
	uint8_t imgSize;

	void processSetCommands(String commandLine, ReadWriteStream& commandOutput);

	void setFormat(uint8_t type);
	void showSettings(ReadWriteStream& commandOutput);

	const char* getImageType();
	const char* getImageSize();
};
