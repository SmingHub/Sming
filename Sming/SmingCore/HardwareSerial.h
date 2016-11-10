/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/**	@defgroup serial Hardware serial
 *	@brief	Hardware serial UARTs
 *  @{
 */

#ifndef _HARDWARESERIAL_H_
#define _HARDWARESERIAL_H_

#include "../Wiring/WiringFrameworkDependencies.h"
#include "../Wiring/Stream.h"
#include "../SmingCore/Delegate.h"
#include "../Services/CommandProcessing/CommandProcessingIncludes.h"

#define UART_ID_0   0 ///< ID of UART 0
#define UART_ID_1   1 ///< ID of UART 1

#define NUMBER_UARTS 2 ///< Quantity of UARTs available

#define SERIAL_SIGNAL_DELEGATE	0 ///< ID for serial delegate signals
#define SERIAL_SIGNAL_COMMAND	1 ///< ID for serial command signals
#define SERIAL_QUEUE_LEN		10 ///< Size of serial queue

/** @brief  Delegate callback type for serial data reception
 *  @param  source Reference to serial stream
 *  @param  arrivedChar Char recieved
 *  @param  availableCharsCount Quantity of chars available stream in receive buffer
 */
// Delegate constructor usage: (&YourClass::method, this)
typedef Delegate<void(Stream &source, char arrivedChar, uint16_t availableCharsCount)> StreamDataReceivedDelegate;

class CommandExecutor;

/// Hardware serial member data
typedef struct
{
	StreamDataReceivedDelegate HWSDelegate; ///< Delegate callback handler
	bool useRxBuff; ///< True to use receiver buffer
	CommandExecutor* commandExecutor = nullptr; ///< Pointer to command executor (Default: none)
} HWSerialMemberData;

/// Hardware serial class
class HardwareSerial : public Stream
{
public:
    /** @brief  Create instance of a hardware serial port object
     *  @param  uartPort UART number [0 | 1]
     *  @note   A global instance of UART 0 is already defined as Serial
     */
	HardwareSerial(const int uartPort);
	~HardwareSerial() {}

    /** @brief  Initialise the serial port
     *  @param  baud BAUD rate of the serial port (Default: 9600)
     */
	void begin(const uint32_t baud = 9600);

    /** @brief  Get quantity characters available from serial input
     *  @retval int Quantity of characters in receive buffer
     */
	int available();

    /** @brief  Read a character from serial port
     *  @retval int Character read from serial port or -1 if buffer empty
     *  @note   The character is removed from the serial port input buffer
    */
	int read();

	/** @brief  Read a block of characters from serial port
	 *  @param  buf Pointer to buffer to hold received data
	 *  @param  max_len Maximum quantity of characters to read
	 *  @retval int Quantity of characters read
	 */
 	int readMemoryBlock(char* buf, int max_len);

    /** @brief  Read a character from serial port without removing from input buffer
     *  @retval int Character read from serial port or -1 if buffer empty
     *  @note   The character remains in serial port input buffer
     */
	int peek();

	/** @brief  Clear the serial port receive buffer
 	 *  @note   All received data is removed from the serial port buffer
 	 *  @warning This function is not implemented!!!
 	 *  @todo   Implement Serial::flush()
	 */
	void flush();

	/** @brief  write a character to serial port
	 *  @param  oneChar Character to write to the serial port
	 *  @retval size_t Quantity of characters written (always 1)
	 */
	size_t write(uint8_t oneChar);

	using Stream::write;

	//void printf(const char *fmt, ...);
	/** @brief  Configure serial port for system debug output
	 *  @param  enabled True to enable this port for system debug output
	 *  @note   If enabled, port will issue system debug messages
	 */
	void systemDebugOutput(bool enabled);

	/** @brief  Configure serial port for command processing
	 *  @param  reqEnable True to enable command processing
	 *  @note   Command processing provides a CLI to the system
	 *  @see    commandHandler
	 */
	void commandProcessing(bool reqEnable);

	/** @brief  Set handler for received data
	 *  @param  reqCallback Function to handle received data
	 *  @param  useSerialRxBuffer True to use the built-in serial receive buffer
	 */
	void setCallback(StreamDataReceivedDelegate reqCallback, bool useSerialRxBuffer = true);

	/** @brief  Remove handler for received data
	 */
	void resetCallback();

    /** @brief  Interrupt handler for UART0 receive events
     *  @todo   Should HardwareSerial::uart0_rx_intr_handler be private?
     */
	static void IRAM_ATTR uart0_rx_intr_handler(void *para);

	/** @brief  Trigger task for input event
	 *  @param  inputEvent The event to use for trigger
	 *  @todo   Should HardwareSerial::delgateTask be private?
	 */
	static void delegateTask (os_event_t *inputEvent);

private:
	int uart;
	static HWSerialMemberData memberData[NUMBER_UARTS];

	os_event_t * serialQueue;
};

/**	@brief	Global instance of serial port UART0
 *	@note	Use Serial.<i>function</i> to access serial functions
 *	@note	Example:
 *  @code   Serial.begin(115200);
	@endcode
*/
extern HardwareSerial Serial;

/** @} */
#endif /* _HARDWARESERIAL_H_ */
