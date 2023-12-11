#include <SmingCore.h>
#include <CommandProcessing/Utils.h>

CommandProcessing::Handler commandHandler;

namespace
{
bool exampleStatus = true;

// Example Command

void processExampleCommand(String commandLine, ReadWriteStream& commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);

	// First token is "example"
	if(numToken == 1) {
		commandOutput << _F("Example Commands available :") << endl;
		commandOutput << _F("on   : Set example status ON") << endl;
		commandOutput << _F("off  : Set example status OFF") << endl;
		commandOutput << _F("status : Show example status") << endl;
	} else if(commandToken[1] == "on") {
		exampleStatus = true;
		commandOutput << _F("Status ON") << endl;
	} else if(commandToken[1] == "off") {
		exampleStatus = false;
		commandOutput << _F("Status OFF") << endl;
	} else if(commandToken[1] == "status") {
		commandOutput << _F("Example Status is ") << (exampleStatus ? "ON" : "OFF") << endl;
	} else {
		commandOutput << _F("Bad command") << endl;
	}
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // Begin serial output

	// Set verbosity
	Serial.systemDebugOutput(true); // Enable debug output to serial
	commandHandler.setVerbose(true);

	// Register Input/Output streams
	CommandProcessing::enable(commandHandler, Serial);

	commandHandler.registerSystemCommands();
	commandHandler.registerCommand(
		CommandProcessing::Command("example", "Example Command", "Application", processExampleCommand));
}
