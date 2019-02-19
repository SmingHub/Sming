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

/** @brief  Delegate constructor usage: (&YourClass::method, this)
 *  @ingroup event_handlers
 */
typedef Delegate<void(char dbgChar)> DebugPrintCharDelegate; ///<Handler function for debug print

/** @brief  Structure for debug options
 *  @ingroup structures
 */
typedef struct {
	DebugPrintCharDelegate debugDelegate = nullptr; ///< Function to handle debug output
	Stream* debugStream = nullptr;					///< Debug output stream
} DebugOuputOptions;

/** @brief  Debug prefix state
 *  @ingroup constants
 */
typedef enum {
	eDBGnoPrefix = 0, ///< Do not use debug prefix
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
	bool status()
	{
		return started;
	}

	/** @brief  Configure debug to use delegate handler for its output
     *  @param  reqDelegate Function to handle debug output
     *  @note   Disables stream output
     */
	void setDebug(DebugPrintCharDelegate reqDelegate);

	/** @brief  Configures debug to use stream for its output
     *  @param  reqStream Stream for debug output
     *  @note   Disables delegate handler
     */
	void setDebug(Stream& reqStream);

	/* implementation of write for Print Class */
	size_t write(uint8_t) override;

private:
	bool started = false;
	bool useDebugPrefix = true;
	bool newDebugLine = true;
	DebugOuputOptions debugOut;
	void printPrefix();
	void processDebugCommands(String commandLine, CommandOutput* commandOutput);
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
