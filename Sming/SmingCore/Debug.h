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

// selective debug logging extension
#define DEBUG_INDENT_MAX                        16 // logBin max indent
#define DEBUG_HEX_PER_LINE                      16 // logBin number of hex chars per line

#define DEBUG_CLS_MAX                            2 // max num classes, scalable upto 16
#define DEBUG_CLS_0                     0x00000000 // reserved for custom application logging
#define DEBUG_CLS_1                     0x01000000 // reserved for Sming framework logging

#define DEBUG_LVL_MAX                           32 //
#define DEBUG_LVL_00                    0x00000000 // 0
#define DEBUG_LVL_01                    0x00010000 // 1
#define DEBUG_LVL_02                    0x00020000 // 2
#define DEBUG_LVL_03                    0x00030000 // 3
#define DEBUG_LVL_04                    0x00040000 // 4
#define DEBUG_LVL_05                    0x00050000 // 5
#define DEBUG_LVL_06                    0x00060000 // 6
#define DEBUG_LVL_07                    0x00070000 // 7
#define DEBUG_LVL_08                    0x00080000 // 8
#define DEBUG_LVL_09                    0x00090000 // 9
#define DEBUG_LVL_0A                    0x000A0000 // 10
#define DEBUG_LVL_0B                    0x000B0000 // 11
#define DEBUG_LVL_0C                    0x000C0000 // 12
#define DEBUG_LVL_0D                    0x000D0000 // 13
#define DEBUG_LVL_0E                    0x000E0000 // 14
#define DEBUG_LVL_0F                    0x000F0000 // 15
#define DEBUG_LVL_10                    0x00100000 // 16
#define DEBUG_LVL_11                    0x00110000 // 17
#define DEBUG_LVL_12                    0x00120000 // 18
#define DEBUG_LVL_13                    0x00130000 // 19
#define DEBUG_LVL_14                    0x00140000 // 20
#define DEBUG_LVL_15                    0x00150000 // 21
#define DEBUG_LVL_16                    0x00160000 // 22
#define DEBUG_LVL_17                    0x00170000 // 23
#define DEBUG_LVL_18                    0x00180000 // 24
#define DEBUG_LVL_19                    0x00190000 // 25
#define DEBUG_LVL_1A                    0x001A0000 // 26
#define DEBUG_LVL_1B                    0x001B0000 // 27
#define DEBUG_LVL_INFO                  0x001C0000 // 28 1C
#define DEBUG_LVL_WARN                  0x001D0000 // 29 1D
#define DEBUG_LVL_ERROR                 0x001E0000 // 30 1E
#define DEBUG_LVL_CRIT                  0x001F0000 // 31 1F

/** @brief  Delegate constructor usage: (&YourClass::method, this)
 *  @ingroup event_handlers
 */
typedef Delegate<void(char dbgChar)> DebugPrintCharDelegate; ///<Handler function for debug print

/** @brief  Structure for debug options
 *  @ingroup structures
 */
typedef struct
{
	DebugPrintCharDelegate debugDelegate = nullptr; ///< Function to handle debug output
	Stream *debugStream = nullptr; ///< Debug output stream
} DebugOuputOptions;

/** @brief  Debug prefix state
 *  @ingroup constants
 */
typedef enum
{
	eDBGnoPrefix  = 0, ///< Do not use debug prefix
	eDBGusePrefix = 1 ///< Use debug prefix
} eDBGPrefix;

/** @defgroup   debug Debug functions
 *  @brief      Provides debug functions
 *  @{
*/

/** @brief  Provides debug output to stream (e.g. Serial) or delegate function handler.
 *
 *  Debug output may be prefixed with an elapsed timestamp. Use standard print methods to produce debug output.
 *  Sming CLI (command handler) may be enabled to provide control of debug output to end user.
 */
class DebugClass : public Print
{
public:
    /** @brief  Instantiate a debug object
     *  @note   Default output is Serial stream
     */
	DebugClass();
	virtual ~DebugClass();

    /** @brief  Enable control of debug output from CLI command handler output
     */
    void initCommand();

    /** @brief  Start debug output
     */
	void start();

    /** @brief  Stop debug output
     */
	void stop();

    /** @brief  Get debug status
     *  @retval bool True if debug enabled
     */
	bool status();

    /** @brief  Configure debug to use delegate handler for its output
     *  @param  reqDelegate Function to handle debug output
     *  @note   Disables stream output
     */
	void setDebug(DebugPrintCharDelegate reqDelegate);

    /** @brief  Configures debug to use stream for its output
     *  @param  reqStream Stream for debug output
     *  @note   Disables delegate handler
     */
	void setDebug(Stream &reqStream);


    /** @brief  Set debug class levels for logTxt() and logBin()
     *  @param  cls        Debug class, DEBUG_CLS_%
     *  @param  clsLevels  Debug levels bitmap for class, 1 bit per level
     *  @note   Usage:     Debug.logClsLevels(DEBUG_CLS_0, 0x000017FF);
     */
	uint32_t           logClsLevels(uint32_t cls, uint32_t clsLevels);
           
    /** @brief  Get debug class levels
     *  @param  cls        Debug class
     *  @note   Usage:     Debug.logClsLevels();
     */
	uint32_t           logClsLevels(uint32_t cls);

    /** @brief  Set msgId usage for logTxt() and logBin()
     *  @param  printMsgId Debug levels for class, 32 bits
     *  @note   Usage:     Debug.logPrintMsgId(true);
     */
  bool               logPrintMsgId(bool printMsgId);

    /** @brief  Get msgId usage
     *  @note   Usage:     Debug.logPrintMsgId();
     */
  bool               logPrintMsgId() ;

    /** @brief  Log textual data if level is enabled for class
     *  @param  msgId      Debug Class/Level/Sequence, 0xCCLLSSSS
     *  @note              CC   = DEBUG_CLS_%
     *  @note              LL   = DEBUG_LVL_%
     *  @note              SSSS = 0000..FFFF, seqnr to map logging to source code 
     *  @param  fmt        Format string with optional args
     *  @note   Usage:     Debug.logTxt(DEBUG_CLS_0 | DEBUG_LVL_01 | 0x1234, "format %u", parm1);
     */
  void               logTxt(uint32_t msgId, const char* fmt, ...);

    /** @brief  Log binary data if level is enabled for class
     *  @param  msgId      Debug Class/Level/Sequence, 0xCCLLSSSS
     *  @note              CC   = DEBUG_CLS_%
     *  @note              LL   = DEBUG_LVL_%
     *  @note              SSSS = 0000..FFFF, seqnr to map logging to source code 
     *  @param  indent     Print indent for binary data, 0-16
     *  @param  pIn        Pointer to binary data
     *  @param  cbIn       Size of binary data
     *  @param  fmt        Format string with optional args
     *  @note   Usage:     Debug.logBin(DEBUG_CLS_0 | DEBUG_LVL_01 | 0x1234, 2, &byArr, sizeof(byArr), "format %u", parm1);
     */
  void               logBin(uint32_t msgId, uint32_t indent, const void *pIn, uint32_t cbIn, const char* fmt, ...);

private:
	bool started = false;
	bool useDebugPrefix = true;
	bool newDebugLine = true;

	DebugOuputOptions debugOut;
	void printPrefix();
	void processDebugCommands(String commandLine, CommandOutput* commandOutput);

	size_t write(uint8_t);  /* implementation of write for Print Class */

	// selective debug logging extension
  bool               m_printMsgId = true;
  uint32_t           m_clsLevels[DEBUG_CLS_MAX];

  void               _NibbleToHex(uint8_t nibIn, char **ppOut);
  void               _ByteToHex(uint8_t byIn, char **ppOut);
  void               _ByteToAsc(uint8_t byIn, char **ppOut);
};

/**	@brief	Global instance of Debug object
 *	@note	Use Debug.<i>function</i> to access Debug functions
 *	@note	Example:
 *  @code
 *  Debug.start();
 *  Debug.printf("My value is %d", myVal);
	@endcode
*/
extern DebugClass Debug;

/** @} */
#endif /* SMINGCORE_DEBUG_H_ */
