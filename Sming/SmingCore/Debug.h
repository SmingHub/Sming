/*
 * Debug.h
 *
 */

#ifndef SMINGCORE_DEBUG_H_
#define SMINGCORE_DEBUG_H_

#include "HardwareSerial.h"
#include "Clock.h"

// Delegate constructor usage: (&YourClass::method, this)
typedef Delegate<void(char dbgChar)> DebugPrintCharDelegate;

typedef struct
{
	DebugPrintCharDelegate debugDelegate = nullptr;
	Stream *debugStream;
} DebugOuputOptions;

class DebugClass : public Print
{
public:
	DebugClass();
	virtual ~DebugClass();
	void start();
	void stop();
	bool status();
	void setDebug(DebugPrintCharDelegate reqDelegate, bool reqStart = true);
	void setDebug(Stream &reqStream, bool reqStart = true);
	// implementation of virtual from Print
	size_t write(uint8_t);

private:
	static void dbgOutputChar(char c);
	void printHeader();
	static DebugClass *Self;
	bool newDebugLine = true;
	bool started = false;
	bool useDebugPrefix = true;
	DebugOuputOptions debugOut;
};

extern DebugClass Debug;

#endif /* SMINGCORE_DEBUG_H_ */
