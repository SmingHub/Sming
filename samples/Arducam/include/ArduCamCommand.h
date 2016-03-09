/*
 * ArduCamCommand.h
 *
 */

#ifndef ARDUCAM_COMMAND_H_
#define ARDUCAM_COMMAND_H_

#include "WString.h"
#include "../Services/CommandProcessing/CommandProcessingIncludes.h"
#include "../Services/CommandProcessing/CommandOutput.h"
#include "CamSettings.h"

class ArduCamCommand
{
public:
	ArduCamCommand(CamSettings *settings);
	virtual ~ArduCamCommand();
	void initCommand();

private:
	bool status = true;
	CamSettings *settings;
	void processSetCommands(String commandLine, CommandOutput* commandOutput);
	void processOutCommands(String commandLine, CommandOutput* commandOutput);
	void showSettings(CommandOutput* commandOutput);
};


#endif /* SMINGCORE_DEBUG_H_ */
