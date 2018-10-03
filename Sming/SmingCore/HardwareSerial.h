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

#define UART_ID_0 0 ///< ID of UART 0
#define UART_ID_1 1 ///< ID of UART 1

#define NUMBER_UARTS 2 ///< Quantity of UARTs available

#define SERIAL_SIGNAL_DELEGATE 0 ///< ID for serial delegate signals
#define SERIAL_SIGNAL_COMMAND 1  ///< ID for serial command signals
#define SERIAL_QUEUE_LEN 10		 ///< Size of serial queue

/** @brief  Delegate callback type for serial data reception
 *  @param  source Reference to serial stream
 *  @param  arrivedChar Char recieved
 *  @param  availableCharsCount Quantity of chars available stream in receive buffer
 */
// Delegate constructor usage: (&YourClass::method, this)
typedef Delegate<void(Stream& source, char arrivedChar, uint16_t availableCharsCount)> StreamDataReceivedDelegate;

class CommandExecutor;

/// Hardware serial member data
typedef struct {
	StreamDataReceivedDelegate HWSDelegate;		///< Delegate callback handler
	CommandExecutor* commandExecutor = nullptr; ///< Pointer to command executor (Default: none)
} HWSerialMemberData;

enum SerialConfig {
	SERIAL_5N1 = UART_5N1,
	SERIAL_6N1 = UART_6N1,
	SERIAL_7N1 = UART_7N1,
	SERIAL_8N1 = UART_8N1,
	SERIAL_5N2 = UART_5N2,
	SERIAL_6N2 = UART_6N2,
	SERIAL_7N2 = UART_7N2,
	SERIAL_8N2 = UART_8N2,
	SERIAL_5E1 = UART_5E1,
	SERIAL_6E1 = UART_6E1,
	SERIAL_7E1 = UART_7E1,
	SERIAL_8E1 = UART_8E1,
	SERIAL_5E2 = UART_5E2,
	SERIAL_6E2 = UART_6E2,
	SERIAL_7E2 = UART_7E2,
	SERIAL_8E2 = UART_8E2,
	SERIAL_5O1 = UART_5O1,
	SERIAL_6O1 = UART_6O1,
	SERIAL_7O1 = UART_7O1,
	SERIAL_8O1 = UART_8O1,
	SERIAL_5O2 = UART_5O2,
	SERIAL_6O2 = UART_6O2,
	SERIAL_7O2 = UART_7O2,
	SERIAL_8O2 = UART_8O2,
};

enum SerialMode { SERIAL_FULL = UART_FULL, SERIAL_RX_ONLY = UART_RX_ONLY, SERIAL_TX_ONLY = UART_TX_ONLY };

/// Hardware serial class
class HardwareSerial : public Stream
{
public:
	/** @brief  Create instance of a hardware serial port object
     *  @param  uartPort UART number [0 | 1]
     *  @note   A global instance of UART 0 is already defined as Serial
     */
	HardwareSerial(const int uartPort);
	~HardwareSerial();

	/** @brief  Initialise the serial port
     *  @param  baud BAUD rate of the serial port (Default: 9600)
     */
	void begin(const uint32_t baud = 9600)
	{
		begin(baud, SERIAL_8N1, SERIAL_FULL, 1);
	}

	/**
	 * @brief Initialise and set its configuration.
	 * @param SerialConfig can be 5, 6, 7, 8 data bits, odd (O),
	 * 					   even (E), and no (N) parity, and 1 or 2 stop bits.
	 * 		  			   To set the desired mode, call  Serial.begin(baudrate, SERIAL_8N1),
	 * 		  			   Serial.begin(baudrate, SERIAL_6E2), etc.
	 */
	void begin(const uint32_t baud, SerialConfig config)
	{
		begin(baud, config, SERIAL_FULL, 1);
	}

	/**
	 * @brief Initialise, set its configuration and mode.
	 * @param SerialConfig can be 5, 6, 7, 8 data bits, odd (O),
	 * 					   even (E), and no (N) parity, and 1 or 2 stop bits.
	 * 		  			   To set the desired mode, call  Serial.begin(baudrate, SERIAL_8N1),
	 * 		  			   Serial.begin(baudrate, SERIAL_6E2), etc.
	 * @param SerialMode specifies if the UART supports receiving (RX), transmitting (TX) or both (FULL) operations
	 */
	void begin(const uint32_t baud, SerialConfig config, SerialMode mode)
	{
		begin(baud, config, mode, 1);
	}

	void begin(const uint32_t baud, SerialConfig config, SerialMode mode, uint8_t txPin);

	/*
	 * @brief De-inits the current UART if it is already used
	 */
	void end();

	/*
	 * @brief Sets receiving buffer size
	 * @param size_t requested size
	 * @retval size_t actual size
	 */
	size_t setRxBufferSize(size_t size);

	/*
	 * @brief Toggle between use of GPIO13/GPIO15 or GPIO3/GPIO(1/2) as RX and TX
	 * @note UART0 uses pins GPIO1 (TX) and GPIO3 (RX). It may be swapped to GPIO15 (TX) and GPIO13 (RX) by calling .swap() after .begin. C
	 * @note Calling swap again maps UART0 back to GPIO1 and GPIO3.
	 */
	void swap()
	{
		swap(1);
	}

	/*
	 * @brief Toggle between use of GPIO13/GPIO15 or GPIO3/GPIO(1/2) as RX and TX
	 * @param uint8_t Pin number.
	 */
	void swap(uint8_t tx_pin);

	/*
	 * @brief Toggle between use of GPIO1 and GPIO2 as TX on UART 0.
	 * @note: UART 1 can't be used if GPIO2 is used with UART 0!
	 * @note: If UART1 is not used and UART0 is not swapped - TX for UART0 can be mapped to GPIO2 by calling .setTx(2) after
	 * 		  .begin or directly with .begin(baud, config, mode, 2).
	 *
	 */
	void setTx(uint8_t tx_pin);

	/*
	 * @brief Sets the transmission and receiving PINS
	 * @param uint8_t tx Transmission pin number
	 * @param uint8_t rx Receiving pin number
	 * @note UART 0 possible options are (1, 3), (2, 3) or (15, 13)
	 * @note UART 1 allows only TX on 2 if UART 0 is not (2, 3)
	 */
	void pins(uint8_t tx, uint8_t rx);

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
	 */
	void flush();

	/** @brief  write a character to serial port
	 *  @param  oneChar Character to write to the serial port
	 *  @retval size_t Quantity of characters written (always 1)
	 */
	virtual size_t write(uint8_t oneChar)
	{
		return uart_write_char(uart, oneChar);
	}

	using Stream::write;

	/** @brief  write multiple characters to serial port
	 *  @param buffer data to write
	 *  @param size number of characters to write
	 *  @retval size_t Quantity of characters written, may be less than size
	 */
	virtual size_t write(const uint8_t* buffer, size_t size)
	{
		return uart_write(uart, buffer, size);
	}

	/** @brief  Configure serial port for system debug output and redirect output from debugf
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
	 *  @retval bool Returns true if the callback was set correctly
	 */
	bool setCallback(StreamDataReceivedDelegate reqCallback);

	/** @brief  Remove handler for received data
	 */
	void resetCallback();

	/**
	 * @brief  Checks if the current UART can transmit(print) information
	 * @retval bool true if transmitting is allowed
	 */
	bool isTxEnabled(void);

	/**
	 * @brief  Checks if the current UART can receive information
	 * @retval bool true if receiving is allowed
	 */
	bool isRxEnabled(void);

	/**
	 * @brief Get the current baud rate
	 * @retval int baud rate
	 */
	int baudRate(void);

	/**
	 * @brief Operator that returns true if the uart structure is set
	 */
	operator bool() const;

	/*
	 * @brief Returns the location of the searched character
	 * @param char c - character to search for
	 * @retval size_t -1 if not found 0 or positive number otherwise
	 */
	size_t indexOf(char c);

	/*
	 * @brief Returns a pointer to the internal uart object. Use with care.
	 * @retval pointer to uart_t
	 */
	uart_t* getUart()
	{
		return uart;
	}

private:
	int uartNr;
	static HWSerialMemberData memberData[NUMBER_UARTS];

	uart_t* uart = nullptr;
	size_t rxSize;

	static os_event_t* serialQueue;

	static bool init;

	/** @brief  Interrupt handler for UART0 receive events
	 * @param uart_t* pointer to UART structure
	 */
	static void IRAM_ATTR callbackHandler(uart_t* arg);

	/** @brief  Trigger task for input event
	 *  @param  inputEvent The event to use for trigger
	 */
	static void delegateTask(os_event_t* inputEvent);
};

/**	@brief	Global instance of serial port UART0
 *  @note	Use Serial.<i>function</i> to access serial functions
 *	@note	Example:
 *  @code   Serial.begin(115200);
 *  @endcode
 *  @note   Serial uses UART0, which is mapped to pins GPIO1 (TX) and GPIO3 (RX).
 *  @note   Serial may be swapped to GPIO15 (TX) and GPIO13 (RX) by calling Serial.swap() after Serial.begin.
 *  @note   Calling swap again maps UART0 back to GPIO1 and GPIO3.
*/
extern HardwareSerial Serial;

/** @} */
#endif /* _HARDWARESERIAL_H_ */
