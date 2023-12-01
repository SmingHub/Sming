#include <SmingCore.h>
#include <Services/CommandProcessing/Utils.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

CommandProcessing::Handler commandHandler;

namespace
{

HttpServer server;
FtpServer ftp;

Timer msgTimer;

bool exampleStatus = true;

// Example Command

void processExampleCommand(String commandLine, ReadWriteStream& commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);

	if(numToken == 1) {
		commandOutput.printf("Example Commands available : \r\n");
		commandOutput.printf("on   : Set example status ON\r\n");
		commandOutput.printf("off  : Set example status OFF\r\n");
		commandOutput.printf("status : Show example status\r\n");
	} else {
		if(commandToken[1] == "on") {
			exampleStatus = true;
			commandOutput.printf("Status ON\r\n");
		} else if(commandToken[1] == "off") {
			exampleStatus = false;
			commandOutput.printf("Status OFF\r\n");
		} else if(commandToken[1] == "status") {
			String tempString = exampleStatus ? "ON" : "OFF";
			commandOutput.printf("Example Status is %s\r\n", tempString.c_str());
		};
	}
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // Begin serial output

	// Set verbosity
	Serial.systemDebugOutput(true); // Enable debug output to serial
	commandHandler.setVerboseMode(CommandProcessing::Handler::VerboseMode::VERBOSE);

	// Register Input/Output streams
	CommandProcessing::enable(commandHandler, Serial);

	commandHandler.registerSystemCommands();
	commandHandler.registerCommand(CommandProcessing::Command("example", "Example Command", "Application", processExampleCommand));
}
