/*
 * ExampleCommand.h
 *
 */

#pragma once

#include "WString.h"
#include <Services/CommandProcessing/CommandProcessingIncludes.h>

class ExampleCommand
{
public:
	ExampleCommand();
	virtual ~ExampleCommand();
	void initCommand();

private:
	bool status = true;
	void processExampleCommands(String commandLine, CommandOutput* commandOutput);
};
