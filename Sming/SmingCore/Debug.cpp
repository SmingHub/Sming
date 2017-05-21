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
	commandHandler.registerCommand(CommandDelegate("debug","New debug in development","Debug",commandFunctionDelegate(&DebugClass::processDebugCommands,this)));
#endif
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


//----------------------------------------------------------------------------
// selective debug logging extension
//----------------------------------------------------------------------------
#define DEBUG_CLS_SHIFT                         24 //
#define DEBUG_LVL_SHIFT                         16 //

//----------------------------------------------------------------------------
// set class levels for logTxt() and logBin()
//----------------------------------------------------------------------------
uint32_t DebugClass::logClsLevels(uint32_t cls, uint32_t clsLevels)
{
  cls = cls >> DEBUG_CLS_SHIFT;
  return (cls < DEBUG_CLS_MAX) ? (m_clsLevels[cls] = clsLevels) : 0; 
  } // logClsLevels

//----------------------------------------------------------------------------
// get class levels
//----------------------------------------------------------------------------
uint32_t DebugClass::logClsLevels(uint32_t cls) 
{
  cls = cls >> DEBUG_CLS_SHIFT;
  return (cls < DEBUG_CLS_MAX) ? m_clsLevels[cls] : 0; 
  } // logClsLevels

//----------------------------------------------------------------------------
// set printMsgId setting for logTxt() and logBin()
//----------------------------------------------------------------------------
bool DebugClass::logPrintMsgId(bool printMsgId)
{
  return m_printMsgId = printMsgId;
  } // logPrintMsgId

//----------------------------------------------------------------------------
// get printMsgId setting
//----------------------------------------------------------------------------
bool DebugClass::logPrintMsgId() 
{
  return m_printMsgId;
  } // logPrintMsgId

//----------------------------------------------------------------------------
// log textual data if level is enabled for class using logClsLevels()
// msgId = 0xCCLLSSSS where CC   = class, DEBUG_CLS_%
//                          LL   = level, DEBUG_LVL_%
//                          SSSS = sequence number
//----------------------------------------------------------------------------
void DebugClass::logTxt(uint32_t msgId, const char* fmt, ...)
{

  do {
    // exit if level not supported for class
    if ((msgId             ) >= (DEBUG_CLS_MAX << DEBUG_CLS_SHIFT))
      break;

    if ((msgId & 0x00FF0000) >= (DEBUG_LVL_MAX << DEBUG_LVL_SHIFT))
      break;

    if ( !(m_clsLevels[msgId >> DEBUG_CLS_SHIFT] & (0x1 << ((msgId & 0x00FF0000) >> DEBUG_LVL_SHIFT))) )
      break;

    // print msgid in hex
    if (m_printMsgId)
      printf("%08X,", msgId);

    // print textual data
    va_list args;
    va_start(args, fmt);
    printf(fmt, args);
    va_end(args);
    println();
    } while (0);

  } // DebugClass::logTxt

//----------------------------------------------------------------------------
// log binary data if level is enabled for class using logClsLevels()
// msgId = 0xCCLLSSSS where CC   = class, DEBUG_CLS_%
//                          LL   = level, DEBUG_LVL_%
//                          SSSS = sequence number
//----------------------------------------------------------------------------
void DebugClass::logBin(uint32_t msgId, uint32_t indent, const void *pIn, uint32_t cbIn, const char* fmt, ...)
{
  char     szOut[DEBUG_INDENT_MAX + (DEBUG_HEX_PER_LINE * 2) + 1 + DEBUG_HEX_PER_LINE + 1];
  char     *pHex, *pAsc;
  uint8_t  *pByt = (uint8_t*) pIn;
  uint32_t dw;

  do {
    // exit if level not supported for class
    if ((msgId             ) >= (DEBUG_CLS_MAX << DEBUG_CLS_SHIFT))
      break;

    if ((msgId & 0x00FF0000) >= (DEBUG_LVL_MAX << DEBUG_LVL_SHIFT))
      break;

    if ( !(m_clsLevels[msgId >> DEBUG_CLS_SHIFT] & (0x1 << ((msgId & 0x00FF0000) >> DEBUG_LVL_SHIFT))) )
      break;

    // print msgid in hex
    if (m_printMsgId)
      printf("%08X,", msgId);

    // print header line
    va_list args;
    va_start(args, fmt);
    printf(fmt, args);
    va_end(args);
    println();

    // print binary data
    if (indent > DEBUG_INDENT_MAX)
      indent = DEBUG_INDENT_MAX;

    while (cbIn) {
      memset(szOut, 0x20, sizeof(szOut));
      pHex = &szOut[indent];
      pAsc = pHex + (DEBUG_HEX_PER_LINE * 2) + 1;

      for (dw = (DEBUG_HEX_PER_LINE < cbIn) ? DEBUG_HEX_PER_LINE : cbIn; dw; dw--, cbIn--) {
        _ByteToAsc(*pByt,   &pAsc);
        _ByteToHex(*pByt++, &pHex);
        } // for

      // print msgid in hex
      if (m_printMsgId)
        printf("%08X,", msgId);

      // print hex output line
      *pAsc = '\0';
      println(szOut);
      } // while
    } while (0);

  } // DebugClass::logBin

//----------------------------------------------------------------------------
// support functions
//----------------------------------------------------------------------------
void DebugClass::_NibbleToHex(uint8_t nibIn, char **ppOut) 
{
  **ppOut = (nibIn < 0x0A) ? nibIn + '0' : nibIn + '7';
  (*ppOut)++;
  } // DebugClass::_NibbleToHex

void DebugClass:: _ByteToHex(uint8_t byIn, char **ppOut) 
{
  _NibbleToHex(byIn / 16, ppOut);
  _NibbleToHex(byIn % 16, ppOut);
  } // DebugClass::_ByteToHex

void DebugClass:: _ByteToAsc(uint8_t byIn, char **ppOut) 
{
  **ppOut = ((byIn < 32) || (byIn > 127)) ? '.' : byIn;
  (*ppOut)++;
  } // DebugClass::_ByteToAsc


DebugClass Debug;

