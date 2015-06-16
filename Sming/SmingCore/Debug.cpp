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
	printPrefix();
	println("Debug started");
}

void DebugClass::stop()
{
	printPrefix();
	println("Debug stopped");
	started = false;
}

bool DebugClass::status()
{
	return started;
}

void DebugClass::setDebug(DebugPrintCharDelegate reqDelegate, eDBGPrefix reqUsePrefix, bool reqStart /*= true */)
{
	os_install_putc1((void *)DebugClass::dbgOutputChar);
	debugOut.debugStream = nullptr;
	debugOut.debugDelegate = reqDelegate;
	setOptions(reqUsePrefix,reqStart);
}

void DebugClass::setDebug(Stream &reqStream, eDBGPrefix reqUsePrefix, bool reqStart /*= true */)
{
	os_install_putc1((void *)DebugClass::dbgOutputChar);
	debugOut.debugDelegate = nullptr;
	debugOut.debugStream = &reqStream;
	setOptions(reqUsePrefix,reqStart);
}

void DebugClass::setOptions(eDBGPrefix reqUsePrefix, bool reqStart)
{
	useDebugPrefix = reqUsePrefix;
	started = reqStart;
}

void DebugClass::setDebugLevel(int reqLevel)
{
	debugLevel = ((reqLevel < logInfo) || (reqLevel > logDebug)) ? logUndefined : reqLevel;
}

int DebugClass::getDebugLevel()
{
	return debugLevel;
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

void DebugClass::printPrefix()
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
			Self->printPrefix();
			Self->newDebugLine = false;
		}
		Self->write(c);
		if (c == '\n')
		{
			Self->newDebugLine = true;
		}
	}
}

DebugClass Debug;

