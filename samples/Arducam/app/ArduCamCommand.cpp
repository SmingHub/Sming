/*
 * Debug.cpp
 *
 */

#include <ArduCamCommand.h>
#include "CamSettings.h"
#include <Libraries/ArduCAM/ov2640_regs.h>


ArduCamCommand::ArduCamCommand(CamSettings *s) {
	debugf("ArduCamCommand Instantiating");
	settings = s;
}

ArduCamCommand::~ArduCamCommand() {
}

void ArduCamCommand::initCommand() {
	commandHandler.registerCommand(
			CommandDelegate("set", "ArduCAM config commands", "Application",
					commandFunctionDelegate(&ArduCamCommand::processSetCommands,this)));
	commandHandler.registerCommand(
			CommandDelegate("out", "ArduCAM output commands", "Application",
					commandFunctionDelegate(&ArduCamCommand::processOutCommands,this)));
}

void ArduCamCommand::showSettings(CommandOutput* commandOutput) {
	// review settings
		commandOutput->printf("ArduCam Settings:\n");
		commandOutput->printf("    img Type: [%s]\n", settings->getImageType());
		commandOutput->printf("    img Size: [%s]\n", settings->getImageSize());
};

void ArduCamCommand::processSetCommands(String commandLine,
		CommandOutput* commandOutput) {
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);

	if (numToken == 1) {
	// review settings
		showSettings(commandOutput);
	}
	// handle command ->   set
	else if (commandToken[1] == "help") {
		commandOutput->printf("set img 	[bmp|jpeg]\r\n");
		commandOutput->printf("set size [160|176|320|352|640|800|1024|1280|1600]\r\n");
	}

	// handle command ->   set
	else if (commandToken[1] == "img") {
		if (numToken == 3) {
			if (commandToken[2] == "bmp") {
				// TODO: set image size and init cam
				settings->setImageType(BMP);
			} else if (commandToken[2] == "jpg") {
				settings->setImageType(JPEG);
			} else
				commandOutput->printf("invalid image format [%s]\r\n", commandToken[2]);
		} else {
			commandOutput->printf("Syntax: set img [bmp|jpeg]\r\n");
		}
		showSettings(commandOutput);
	}

	// handle command ->   settings
	else if (commandToken[1] == "size") {
		if (numToken == 3) {
			if (commandToken[2] == "160") {
				settings->setImageType(JPEG);
				settings->setImageSize(OV2640_160x120);
			} else if (commandToken[2] == "176") {
				settings->setImageType(JPEG);
				settings->setImageSize(OV2640_176x144);
			} else if (commandToken[2] == "320") {
				settings->setImageType(JPEG);
				settings->setImageSize(OV2640_320x240);
			} else if (commandToken[2] == "352") {
				settings->setImageType(JPEG);
				settings->setImageSize(OV2640_352x288);
			} else if (commandToken[2] == "640") {
				settings->setImageType(JPEG);
				settings->setImageSize(OV2640_640x480);
			} else if (commandToken[2] == "800") {
				settings->setImageType(JPEG);
				settings->setImageSize(OV2640_800x600);
			} else if (commandToken[2] == "1024") {
				settings->setImageType(JPEG);
				settings->setImageSize(OV2640_1024x768);
			} else if (commandToken[2] == "1280") {
				settings->setImageType(JPEG);
				settings->setImageSize(OV2640_1280x1024);
			} else if (commandToken[2] == "1600") {
				settings->setImageType(JPEG);
				settings->setImageSize(OV2640_1600x1200);
			} else {
				commandOutput->printf("invalid size definition[%s]\r\n", commandToken[2]);
			}
		} else {
			commandOutput->printf("Syntax: set size [160|176|320|352|640|800|1024|1280|1600]\r\n");
		}
		showSettings(commandOutput);
	}
}

void ArduCamCommand::processOutCommands(String commandLine,
		CommandOutput* commandOutput) {
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);

	if (numToken == 1) {
	// review settings
		commandOutput->printf("handle output commands\r\n");
	}
}



