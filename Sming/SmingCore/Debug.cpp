/*
 * Debug.cpp
 *
 */

#include <Debug.h>

DebugClass *DebugClass::Self;

DebugClass::DebugClass()
{
	debugf("DebugClass Start instantiating");
	Self = this;
}

DebugClass::~DebugClass()
{
}

void DebugClass::start()
{
	started = true;
	printHeader();
	println("Debug started");
}

void DebugClass::stop()
{
	printHeader();
	println("Debug stopped");
	started = false;
}

bool DebugClass::status()
{
	return started;
}

void DebugClass::setDebug(DebugPrintCharDelegate reqDelegate, bool reqStart /*= true */)
{
	os_install_putc1((void *)DebugClass::dbgOutputChar);
	debugOut.debugStream = nullptr;
	debugOut.debugDelegate = reqDelegate;
	if (reqStart) {started = true;}
}

void DebugClass::setDebug(Stream &reqStream, bool reqStart /*= true */)
{
	os_install_putc1((void *)DebugClass::dbgOutputChar);
	debugOut.debugDelegate = nullptr;
	debugOut.debugStream = &reqStream;
	if (reqStart) {started = true;}
}

size_t DebugClass::write(uint8_t c)
{
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
	return 0;
}

void DebugClass::printHeader()
{
	if (useDebugPrefix)
	{
		uint32_t curMillis = millis();
		printf("Dbg %4d.%03d : ", curMillis/1000, curMillis % 1000);
	}
}

void DebugClass::dbgOutputChar(char c)
{
	if (Self->started)
	{
		if (Self->newDebugLine)
		{
			Self->printHeader();
			Self->newDebugLine = false;
		}
		Self->write(c);
		if (c == '\n')
		{
			Self->newDebugLine = true;
			Self->print("!!");
		}
	}
}

DebugClass Debug;

