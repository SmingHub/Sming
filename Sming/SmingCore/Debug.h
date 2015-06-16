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
	void start();
	void stop();
	bool status();
	void setOptions(eDBGPrefix reqUsePrefix, bool reqStart);
	void setDebug(DebugPrintCharDelegate reqDelegate, eDBGPrefix reqUsePrefix, bool reqStart = true);
	void setDebug(Stream &reqStream, eDBGPrefix reqUsePrefix, bool reqStart = true);

	template <typename... Args>
	size_t lprintf(int level, const char* fmt, Args... args);

	// implementation of virtual from Print
	size_t write(uint8_t);

private:
	static void dbgOutputChar(char c);
	void printPrefix();
	static DebugClass *Self;
	bool newDebugLine = true;
	bool started = false;
	bool useDebugPrefix = true;
	DebugOuputOptions debugOut;
};

extern DebugClass Debug;

template <typename... Args>
size_t lprintf2(int level, const char* fmt, Args... args)
{
	if (level == 0)
	{
		return 0;
	}
	else
	{
		return Debug.printf(fmt, args...);
	}
}

#endif /* SMINGCORE_DEBUG_H_ */
