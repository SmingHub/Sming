/*
 * ArduCamCommand.h
 *
 */

#ifndef ARDUCAM_COMMAND_H_
#define ARDUCAM_COMMAND_H_

#include "WString.h"
#include "../Services/CommandProcessing/CommandProcessingIncludes.h"
#include "../Services/CommandProcessing/CommandOutput.h"
#include <Libraries/ArduCam/ArduCAM.h>

class ArduCamCommand
{
public:
	ArduCamCommand(ArduCAM *CAM);
	virtual ~ArduCamCommand();
	void initCommand();
	const char * getContentType();

private:
	bool status = true;
	ArduCAM *myCAM;
	uint8	imgType;
	uint8	imgSize;
	void processSetCommands(String commandLine, CommandOutput* commandOutput);

	void set_format(uint8 type);
	void showSettings(CommandOutput* commandOutput);

	const char * getImageType();
	const char * getImageSize();
};


#endif /* SMINGCORE_DEBUG_H_ */
