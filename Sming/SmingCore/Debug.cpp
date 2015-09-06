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
	commandHandler.registerCommand(CommandDelegate("debug","New debug in development","Debug",commandFunctionDelegate(&DebugClass::processDebugCommands,this)));
}

void DebugClass::start()
{
	started = true;
	println("Debug started");
}

void DebugClass::stop()
{
	println("Debug stopped");
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
	printf("Welcome to DebugDelegate\r\n");
}

void DebugClass::setDebug(Stream &reqStream)
{
	debugOut.debugDelegate = nullptr;
	debugOut.debugStream = &reqStream;
	printf("Welcome to DebugStream");
}

void DebugClass::printPrefix()
{
	if (useDebugPrefix)
	{
		uint32_t curMillis = millis();
		printf("Dbg %4d.%03d : ", curMillis/1000, curMillis % 1000);
	}
}

size_t DebugClass::write(uint8_t c)
{
	if (started)
	{
		if (newDebugLine)
		{
			newDebugLine = false;
			printPrefix();
		}
		if (c == '\n')
		{
			newDebugLine = true;
		}
		if (debugOut.debugDelegate)
		{
			debugOut.debugDelegate(c);
			return 1;
		}
		if (debugOut.debugStream)
		{
			debugOut.debugStream->write(c);
			return 1;
		}
	}

	return 0;
}

void DebugClass::processDebugCommands(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ' , commandToken);

	if (numToken == 1)
	{
		commandOutput->printf("Debug Commands available : \r\n");
		commandOutput->printf("on   : Start Debug output\r\n");
		commandOutput->printf("off  : Stop Debug output\r\n");
		commandOutput->printf("serial : Send Debug output to Serial\r\n");
	}
	else
	{
		if (commandToken[1] == "on")
		{
			start();
			commandOutput->printf("Debug started\r\n");
		}
		else if (commandToken[1] == "off")
		{
			commandOutput->printf("Debug stopped\r\n");
			stop();
		}
		else if (commandToken[1] == "serial")
		{
			setDebug(Serial);
			commandOutput->printf("Debug set to Serial");
		};

	}
}

DebugClass Debug;

