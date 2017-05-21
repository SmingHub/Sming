
//----------------------------------------------------------------------------
// Sample to demonstrate selective debug logging for the Sming Debug object
//
// For now I added 2 log classes (DEBUG_CLS_0 and DEBUG_CLS_1), where class 0
// is intended for logging created by an application using the Sming framework,
// and class 1 is intended for the Sming framework itself.
//
// 
// 
// Per class you have 32 log levels, they have a name and a mask like this:
//   level 0  : name = DEBUG_LVL_00     -> mask = 0x00000001
//   level 1  : name = DEBUG_LVL_01     -> mask = 0x00000002
//   ...
//   level 26 : name = DEBUG_LVL_1A     -> mask = 0x04000000
//   level 27 : name = DEBUG_LVL_1B     -> mask = 0x08000000
//   level 28 : name = DEBUG_LVL_INFO   -> mask = 0x10000000
//   level 29 : name = DEBUG_LVL_WARN   -> mask = 0x20000000
//   level 30 : name = DEBUG_LVL_ERROR  -> mask = 0x40000000
//   level 31 : name = DEBUG_LVL_CRIT   -> mask = 0x80000000
//
// To enable levels for a class, you set their OR'ed mask value to the Debug
// object using the logClsLevels(cls, clsLevels) function. 
//
// I.e. to enable levels 0, 1, 8 and 16 you OR their mask values like this:
//   Debug.logClsLevels(DEBUG_CLS_0, 0x00010103)
//
// Use Debug.logClsLevels(cls) to read the current levels set for a class.
//   Debug.logClsLevels(DEBUG_CLS_0)
// 
//
//
// Using logPrintMsgId(bool) you can enable/disable printing of the msgId in front
// of the logged line(s), by default this is disabled. I.e. 
//   Debug.logPrintMsgId(true);
//   Debug.logPrintMsgId(false);
//
// Use logPrintMsgId() to obtain the current setting.
//   Debug.logPrintMsgId();
//
// 
//
// The msgId is a 32bit value and the first parameter to logTxt() and logBin(), it
// is composed by OR'ing the class, level and an optional sequence number like this:
//   DEBUG_CLS_0 | DEBUG_LVL_00 | 0x0010
//
// Since DEBUG_CLS_0 evaluates to 0 it can be omitted.
//
// The level is used by logTxt() and logBin() to check whether that level is enabled
// for the given class, only if enabled will logTxt() or logBin() generate logging.
//
// The sequence number can be used to identify the exact location in code that 
// generates a certain logging line, it can vary between 0x0000 and 0xFFFF.
// 
//
//
// Using logTxt(msgId, format, ....) you can log strings as if using printf, i.e.
//   Debug.logTxt(DEBUG_LVL_05 | 0x0000, "appOnPublish,topic=%s,msg=%s", strTopic.c_str(), strMsg.c_str());
//
// Might show:
//   Dbg 1787.887 : appOnPublish,topic=astr76b32/node0/cmd/out0,msg=ontimed.5000
//
// Or this after Debug.logPrintMsgId(true):
//   Dbg 1719.666 : 00050000,appOnPublish,topic=astr76b32/node0/cmd/out0,msg=ontimed.5000
//
// 
//
// Using logBin(msgId, indent, pBuf, cbBuf, format, ...) you can log a buffer of data in hex format, i.e.
//   Debug.logBin(DEBUG_LVL_05 | 0x0000, 2, strTopic.c_str(), strTopic.length(),
//                "appOnPublish,topic=%s,msg=%s", strTopic.c_str(), strMsg.c_str());
//
// Might show:
//   Dbg 1787.894 : appOnPublish,topic=astr76b32/node0/cmd/out0,msg=ontimed.5000
//   Dbg 1787.896 :   6173747237366233322F6E6F6465302F astr76b32/node0/
//   Dbg 1787.902 :   636D642F6F757430                 cmd/out0
//
// Or this after Debug.logPrintMsgId(true):
//   Dbg 1719.673 : 00050010,appOnPublish,topic=astr76b32/node0/cmd/out0,msg=ontimed.5000
//   Dbg 1719.677 : 00050010,  6173747237366233322F6E6F6465302F astr76b32/node0/
//   Dbg 1719.683 : 00050010,  636D642F6F757430                 cmd/out0
//
//
//
// Using this feature you can develop your application with the neccesary 
// logging code embedded in the product. You can enabled logging selective to
// help debugging during development or troubleshooting at a customer location.
//
// Performance or memory overhead will be very limited.
//----------------------------------------------------------------------------
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Debug.h>

//----------------------------------------------------------------------------
#define CLSLVL_APP                    DEBUG_LVL_00 // 0x0001
#define CLSLVL_EVEN                   DEBUG_LVL_01 // 0x0002
#define CLSLVL_ODD                    DEBUG_LVL_02 // 0x0004

Timer   g_timerApp;
int     g_nCount = 0;


//----------------------------------------------------------------------------
// application function 1
//----------------------------------------------------------------------------
void appFnEven(uint8_t* pIn, int cbIn, const char* szTitle)
{
  Debug.logTxt(CLSLVL_EVEN | 0x0000, "appFnEven,cbIn=%d", cbIn);
  Debug.logBin(CLSLVL_EVEN | 0x0010, 2, pIn, cbIn, "appFnEven,title=%s", szTitle);
  } // appFnEven

//----------------------------------------------------------------------------
// application function 2
//----------------------------------------------------------------------------
void appFnOdd(uint8_t* pIn, int cbIn, const char* szTitle)
{
  Debug.logTxt(CLSLVL_EVEN | 0x0000, "appFnOdd,cbIn=%d", cbIn);
  Debug.logBin(CLSLVL_EVEN | 0x0010, 2, pIn, cbIn, "appFnOdd,title=%s", szTitle);
  } // appFnOdd

//----------------------------------------------------------------------------
// application timer callback
//----------------------------------------------------------------------------
void appTimerCb()
{
  uint8_t buf[64];
  int     n;
   
  // prepare buffer to log
  Debug.logTxt(CLSLVL_APP | 0x0000, "appTimerCb");
  g_nCount++;
  memset(buf, (uint8_t)(g_nCount % 256), sizeof(buf));

  if (g_nCount % 2)
    appFnOdd(buf, sizeof(buf), "This is the odd call");
  else 
    appFnEven(buf, sizeof(buf), "This is the even call");

  Debug.logTxt(CLSLVL_APP | 0x9999, "appTimerCb");
  } // appTimerCb

//----------------------------------------------------------------------------
// main entry
//----------------------------------------------------------------------------
void init()
{
  // enable printing of msgId in front of logline
  Debug.logPrintMsgId(true);

  // set class 0 loglevels for DEBUG_LVL_00, DEBUG_LVL_01 and DEBUG_LVL_02
  Debug.logClsLevels(DEBUG_CLS_0, 0x00000007);

  // start app timer
  g_timerApp.initializeMs(1000, appTimerCb).start();
  } // init
