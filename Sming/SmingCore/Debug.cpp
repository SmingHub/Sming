/*
 * Debug.cpp
 *
 */

#include <Debug.h>

DebugClass::DebugClass()
{
	debugf("DebugClass Instantiating");
	setDebug(Serial);
}

DebugClass::~DebugClass()
{
}

void DebugClass::initCommand()
{
#if ENABLE_CMD_EXECUTOR
	auto debug = F("Debug");
	commandHandler.registerCommand(CommandDelegate(debug, F("New debug in development"), debug,
												   commandFunctionDelegate(&DebugClass::processDebugCommands, this)));
#endif
}

void DebugClass::start()
{
	started = true;
	println(_F("Debug started"));
}

void DebugClass::stop()
{
	println(_F("Debug stopped"));
	started = false;
}

bool DebugClass::status()
{
	return started;
}

void DebugClass::setDebug(DebugPrintCharDelegate reqDelegate)
{
	debugOut.debugStream = nullptr;
	debugOut.debugDelegate = reqDelegate;
	print(_F("Welcome to DebugDelegate\r\n"));
}

void DebugClass::setDebug(Stream& reqStream)
{
	debugOut.debugDelegate = nullptr;
	debugOut.debugStream = &reqStream;
	print(_F("Welcome to DebugStream"));
}

void DebugClass::printPrefix()
{
	if(useDebugPrefix) {
		uint32_t curMillis = millis();
		printf(_F("Dbg %4u.%03u : "), curMillis / 1000, curMillis % 1000);
	}
}

size_t DebugClass::write(uint8_t c)
{
	if(started) {
		if(newDebugLine) {
			newDebugLine = false;
			printPrefix();
		}
		if(c == '\n') {
			newDebugLine = true;
		}
		if(debugOut.debugDelegate) {
			debugOut.debugDelegate(c);
			return 1;
		}
		if(debugOut.debugStream) {
			debugOut.debugStream->write(c);
			return 1;
		}
	}

	return 0;
}

void DebugClass::processDebugCommands(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);

	if(numToken == 1) {
		commandOutput->print(_F("Debug Commands available : \r\n"));
		commandOutput->print(_F("on   : Start Debug output\r\n"));
		commandOutput->print(_F("off  : Stop Debug output\r\n"));
		commandOutput->print(_F("serial : Send Debug output to Serial\r\n"));
	} else {
		if(commandToken[1] == "on") {
			start();
			commandOutput->print(_F("Debug started\r\n"));
		} else if(commandToken[1] == _F("off")) {
			commandOutput->print(_F("Debug stopped\r\n"));
			stop();
		} else if(commandToken[1] == _F("serial")) {
			setDebug(Serial);
			commandOutput->print(_F("Debug set to Serial"));
		};
	}
}

DebugClass Debug;
