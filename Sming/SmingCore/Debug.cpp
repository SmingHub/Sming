/*
 * Debug.cpp
 *
 */

#include "Debug.h"
#include "HardwareSerial.h"
#include "Clock.h"

DebugClass Debug;

DebugClass::DebugClass()
{
	debugf("DebugClass Instantiating");
	setDebug(Serial);
}

DebugClass::~DebugClass()
{}

void DebugClass::initCommand()
{
#if ENABLE_CMD_EXECUTOR
	commandHandler.registerCommand(CommandDelegate("debug", "New debug in development", "Debug",
												   commandFunctionDelegate(&DebugClass::processDebugCommands, this)));
#endif
}

void DebugClass::start()
{
	_started = true;
	println("Debug started");
}

void DebugClass::stop()
{
	println("Debug stopped");
	_started = false;
}

bool DebugClass::status()
{
	return _started;
}

void DebugClass::setDebug(DebugPrintCharDelegate reqDelegate)
{
	_debugOut.debugStream = nullptr;
	_debugOut.debugDelegate = reqDelegate;
	printf("Welcome to DebugDelegate\r\n");
}

void DebugClass::setDebug(Stream& reqStream)
{
	_debugOut.debugDelegate = nullptr;
	_debugOut.debugStream = &reqStream;
	printf("Welcome to DebugStream");
}

void DebugClass::printPrefix()
{
	if (_useDebugPrefix) {
		uint32_t curMillis = millis();
		printf("Dbg %4d.%03d : ", curMillis / 1000, curMillis % 1000);
	}
}

size_t DebugClass::write(uint8_t c)
{
	if (!_started)
		return 0;

	if (_newDebugLine) {
		_newDebugLine = false;
		printPrefix();
	}

	if (c == '\n')
		_newDebugLine = true;

	if (_debugOut.debugDelegate) {
		_debugOut.debugDelegate(c);
		return 1;
	}

	if (_debugOut.debugStream) {
		_debugOut.debugStream->write(c);
		return 1;
	}

	return 0;
}

void DebugClass::processDebugCommands(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandTokens;
	unsigned numToken = commandLine.split(' ', commandTokens);

	if (numToken == 1) {
		commandOutput->print(_F("Debug Commands available : \r\n"));
		commandOutput->print(_F("on   : Start Debug output\r\n"));
		commandOutput->print(_F("off  : Stop Debug output\r\n"));
		commandOutput->print(_F("serial : Send Debug output to Serial\r\n"));
	}
	else if (commandTokens[1] == "on") {
		start();
		commandOutput->print(_F("Debug started\r\n"));
	}
	else if (commandTokens[1] == "off") {
		commandOutput->print(_F("Debug stopped\r\n"));
		stop();
	}
	else if (commandTokens[1] == "serial") {
		setDebug(Serial);
		commandOutput->print(_F("Debug set to Serial"));
	}
}
