
#include <ArduCamCommand.h>
#include <Libraries/ArduCAM/ArduCAM.h>
#include <Libraries/ArduCAM/ov2640_regs.h>

ArduCamCommand::ArduCamCommand(ArduCAM* CAM)
{
	debugf("ArduCamCommand Instantiating");
	myCAM = CAM;
	imgSize = OV2640_320x240;
	imgType = JPEG;
}

ArduCamCommand::~ArduCamCommand()
{
}

void ArduCamCommand::initCommand()
{
	commandHandler.registerCommand(CommandDelegate("set", "ArduCAM config commands", "Application",
												   CommandFunctionDelegate(&ArduCamCommand::processSetCommands, this)));
	//	commandHandler.registerCommand(
	//			CommandDelegate("out", "ArduCAM output commands", "Application",
	//					CommandFunctionDelegate(&ArduCamCommand::processOutCommands,this)));
}

void ArduCamCommand::showSettings(CommandOutput* commandOutput)
{
	// review settings
	commandOutput->printf("ArduCam Settings:\n");
	commandOutput->printf("    img Type: [%s]\n", getImageType());
	commandOutput->printf("    img Size: [%s]\n", getImageSize());
};

void ArduCamCommand::processSetCommands(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);

	if(numToken == 1) {
		// review settings
		showSettings(commandOutput);
	}
	// handle command ->   set
	else if(commandToken[1] == "help") {
		commandOutput->printf("set img 	[bmp|jpeg]\r\n");
		commandOutput->printf("set size [160|176|320|352|640|800|1024|1280|1600]\r\n");
	}

	// handle command ->   set
	else if(commandToken[1] == "img") {
		if(numToken == 3) {
			if(commandToken[2] == "bmp") {
				// TODO: set image size and init cam
				//				settings->setImageType(BMP);
				set_format(BMP);
			} else if(commandToken[2] == "jpg") {
				set_format(JPEG);
			} else
				commandOutput->printf("invalid image format [%s]\r\n", commandToken[2].c_str());
		} else {
			commandOutput->printf("Syntax: set img [bmp|jpeg]\r\n");
		}
		showSettings(commandOutput);
	}

	// handle command ->   settings
	else if(commandToken[1] == "size") {
		if(numToken == 3) {
			if(commandToken[2] == "160") {
				imgSize = OV2640_160x120;
				myCAM->OV2640_set_JPEG_size(OV2640_160x120);
				set_format(JPEG);
			} else if(commandToken[2] == "176") {
				imgSize = OV2640_176x144;
				myCAM->OV2640_set_JPEG_size(OV2640_176x144);
				set_format(JPEG);
			} else if(commandToken[2] == "320") {
				imgSize = OV2640_320x240;
				myCAM->OV2640_set_JPEG_size(OV2640_320x240);
			} else if(commandToken[2] == "352") {
				imgSize = OV2640_352x288;
				myCAM->OV2640_set_JPEG_size(OV2640_352x288);
				set_format(JPEG);
			} else if(commandToken[2] == "640") {
				imgSize = OV2640_640x480;
				myCAM->OV2640_set_JPEG_size(OV2640_640x480);
				set_format(JPEG);
			} else if(commandToken[2] == "800") {
				imgSize = OV2640_800x600;
				myCAM->OV2640_set_JPEG_size(OV2640_800x600);
				set_format(JPEG);
			} else if(commandToken[2] == "1024") {
				imgSize = OV2640_1024x768;
				myCAM->OV2640_set_JPEG_size(OV2640_1024x768);
				set_format(JPEG);
			} else if(commandToken[2] == "1280") {
				imgSize = OV2640_1280x1024;
				myCAM->OV2640_set_JPEG_size(OV2640_1280x1024);
				set_format(JPEG);
			} else if(commandToken[2] == "1600") {
				imgSize = OV2640_1600x1200;
				myCAM->OV2640_set_JPEG_size(OV2640_1600x1200);
				set_format(JPEG);
			} else {
				commandOutput->printf("invalid size definition[%s]\r\n", commandToken[2].c_str());
			}
		} else {
			commandOutput->printf("Syntax: set size [160|176|320|352|640|800|1024|1280|1600]\r\n");
		}
		showSettings(commandOutput);
	}
}

void ArduCamCommand::set_size(String size)
{
	if(size == "160x120") {
		imgSize = OV2640_160x120;
		myCAM->OV2640_set_JPEG_size(OV2640_160x120);
		set_format(JPEG);
	} else if(size == "176x144") {
		imgSize = OV2640_176x144;
		myCAM->OV2640_set_JPEG_size(OV2640_176x144);
		set_format(JPEG);
	} else if(size == "320x240") {
		imgSize = OV2640_320x240;
		myCAM->OV2640_set_JPEG_size(OV2640_320x240);
	} else if(size == "352x288") {
		imgSize = OV2640_352x288;
		myCAM->OV2640_set_JPEG_size(OV2640_352x288);
		set_format(JPEG);
	} else if(size == "640x480") {
		imgSize = OV2640_640x480;
		myCAM->OV2640_set_JPEG_size(OV2640_640x480);
		set_format(JPEG);
	} else if(size == "800x600") {
		imgSize = OV2640_800x600;
		myCAM->OV2640_set_JPEG_size(OV2640_800x600);
		set_format(JPEG);
	} else if(size == "1024x768") {
		imgSize = OV2640_1024x768;
		myCAM->OV2640_set_JPEG_size(OV2640_1024x768);
		set_format(JPEG);
	} else if(size == "1280x1024") {
		imgSize = OV2640_1280x1024;
		myCAM->OV2640_set_JPEG_size(OV2640_1280x1024);
		set_format(JPEG);
	} else if(size == "1600x1200") {
		imgSize = OV2640_1600x1200;
		myCAM->OV2640_set_JPEG_size(OV2640_1600x1200);
		set_format(JPEG);
	} else {
		debugf("ERROR: invalid size definition[%s]\r\n", size.c_str());
	}
}

void ArduCamCommand::set_type(String type)
{
	if(type == "BMP") {
		myCAM->set_format(BMP);
		if(imgType != BMP) {
			// reset the cam
			myCAM->InitCAM();
			imgType = BMP;
			imgSize = OV2640_320x240;
		}
	} else {
		myCAM->set_format(JPEG);
		// reset the cam
		if(imgType != JPEG) {
			// reset the cam
			myCAM->InitCAM();
			myCAM->OV2640_set_JPEG_size(imgSize);
			imgType = JPEG;
		}
	}
}

void ArduCamCommand::set_format(uint8 type)
{
	if(type == BMP) {
		myCAM->set_format(BMP);
		if(imgType != BMP) {
			// reset the cam
			myCAM->InitCAM();
			imgType = BMP;
			imgSize = OV2640_320x240;
		}
	} else {
		myCAM->set_format(JPEG);
		// reset the cam
		if(imgType != JPEG) {
			// reset the cam
			myCAM->InitCAM();
			myCAM->OV2640_set_JPEG_size(imgSize);
			imgType = JPEG;
		}
	}
}

const char* ArduCamCommand::getImageType()
{
	switch(imgType) {
	case JPEG:
		return "JPEG";
	case BMP:
	default:
		return "BMP";
	}
}

const char* ArduCamCommand::getContentType()
{
	switch(imgType) {
	case JPEG:
		return "image/jpeg";
	case BMP:
	default:
		return "image/x-ms-bmp";
	}
}

const char* ArduCamCommand::getImageSize()
{
	switch(imgSize) {
	case OV2640_1600x1200:
		return "1600x1200";
	case OV2640_1280x1024:
		return "1280x1024";
	case OV2640_1024x768:
		return "1024x768";
	case OV2640_800x600:
		return "800x600";
	case OV2640_640x480:
		return "640x480";
	case OV2640_352x288:
		return "352x288";
	case OV2640_320x240:
		return "320x240";
	case OV2640_176x144:
		return "176x144";
	case OV2640_160x120:
		return "160x120";
	default:
		return "320x240";
	}
}
