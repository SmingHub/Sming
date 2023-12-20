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
	virtual ~ArduCamCommand();
	void initCommand();
	const char* getContentType();
	void setSize(const String& size);
	void setType(const String& type);

private:
	bool status = true;
	ArduCAM myCAM;
	CommandProcessing::Handler* commandHandler{nullptr};
	uint8 imgType;
	uint8 imgSize;
	void processSetCommands(String commandLine, ReadWriteStream& commandOutput);

	void setFormat(uint8 type);
	void showSettings(ReadWriteStream& commandOutput);

	const char* getImageType();
	const char* getImageSize();
};
