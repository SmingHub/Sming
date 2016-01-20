/*
 * Debug.h
 *
 */
/** @defgroup   debug Debug functions
 *  @brief      Provides debug functions
*/

#ifndef SMINGCORE_DEBUG_H_
#define SMINGCORE_DEBUG_H_

#include "HardwareSerial.h"
#include "Clock.h"
#include "WString.h"
#include "../Services/CommandProcessing/CommandProcessingIncludes.h"

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

class DebugClass : public Print
{
public:
    /** @brief  Debug class
     *  @addtogroup debug
     *  @{
     */
	DebugClass();
	virtual ~DebugClass();

    /** @brief  Initialise debug output
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

    /** @brief  Set debug handler
     *  @param  reqDelegate Function to handle debug output
     */
	void setDebug(DebugPrintCharDelegate reqDelegate);

    /** @brief  Set debug stream
     *  @param  reqStream Stream for debug output
     */
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

/**	@brief	Global instance of Debug object
 *	@note	Use Debug.<i>function</i> to access Debug functions
 *	@note	Example:
 *  @code   Debug.start();
	@endcode
*/
extern DebugClass Debug;

/** @} */
#endif /* SMINGCORE_DEBUG_H_ */
