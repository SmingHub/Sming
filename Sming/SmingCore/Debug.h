/*
 * Debug.h
 *
 */

#ifndef SMINGCORE_DEBUG_H_
#define SMINGCORE_DEBUG_H_

#include "HardwareSerial.h"
#include "Clock.h"
#include "WString.h"
#include "../Services/CommandProcessing/CommandProcessingIncludes.h"

// Delegate constructor usage: (&YourClass::method, this)
typedef Delegate<void(char dbgChar)> DebugPrintCharDelegate;

typedef struct
{
	DebugPrintCharDelegate debugDelegate = nullptr;
	Stream *debugStream = nullptr;
} DebugOuputOptions;

typedef enum
{
	eDBGnoPrefix  = 0,
	eDBGusePrefix = 1
} eDBGPrefix;

class DebugClass : public Print
{
public:
	DebugClass();
	virtual ~DebugClass();
	void initCommand();
	void start();
	void stop();
	bool status();
	void setDebug(DebugPrintCharDelegate reqDelegate);
	void setDebug(Stream &reqStream);

private:
	bool started = false;
	bool useDebugPrefix = true;
	bool newDebugLine = true;
	DebugOuputOptions debugOut;
	void printPrefix();
	void processDebugCommands(String commandLine, CommandOutput* commandOutput);

	size_t write(uint8_t);  /* implementation of write for Print Class */
};

extern DebugClass Debug;

#endif /* SMINGCORE_DEBUG_H_ */
