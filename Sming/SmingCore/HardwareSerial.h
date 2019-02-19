/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HardwareSerial.h
 *
 ****/

/**	@defgroup serial Hardware serial
 *	@brief	Hardware serial UARTs
 *  @{
 */

#ifndef _SMING_CORE_HARDWARE_SERIAL_H_
#define _SMING_CORE_HARDWARE_SERIAL_H_

#include "WiringFrameworkDependencies.h"
#include "Data/Stream/ReadWriteStream.h"
#include "Delegate.h"
#include "espinc/uart.h"

#define UART_ID_0 0 ///< ID of UART 0
#define UART_ID_1 1 ///< ID of UART 1

#define NUMBER_UARTS 2 ///< Quantity of UARTs available

class HardwareSerial;

/** @brief  Delegate callback type for serial data reception
 *  @param  source Reference to serial stream
 *  @param  arrivedChar Char received
 *  @param  availableCharsCount Quantity of chars available stream in receive buffer
 *  @note Delegate constructor usage: (&YourClass::method, this)
 *
 * 	This delegate is invoked when the serial receive buffer is full, or it times out. The
 * 	arrivedChar indicates the last character received, which might be a '\n' line ending
 * 	character, for example.
 *
 * 	If no receive buffer has been allocated, or it's not big enough to contain the full message,
 * 	then this value will be incorrect as data is stored in the hardware FIFO until read out.
 */
typedef Delegate<void(Stream& source, char arrivedChar, uint16_t availableCharsCount)> StreamDataReceivedDelegate;

/** @brief Delegate callback type for serial data transmit completion
 *  @note Invoked when the last byte has left the hardware FIFO
 */
typedef Delegate<void(HardwareSerial& serial)> TransmitCompleteDelegate;

#if ENABLE_CMD_EXECUTOR
class CommandExecutor;
#endif

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

/** @brief values equivalent to uart_mode_t */
enum SerialMode { SERIAL_FULL = UART_FULL, SERIAL_RX_ONLY = UART_RX_ONLY, SERIAL_TX_ONLY = UART_TX_ONLY };

#ifndef DEFAULT_RX_BUFFER_SIZE
#define DEFAULT_RX_BUFFER_SIZE 256
#endif

#ifndef DEFAULT_TX_BUFFER_SIZE
#define DEFAULT_TX_BUFFER_SIZE 0
#endif

/// Hardware serial class
class HardwareSerial : public ReadWriteStream
{
public:
	/** @brief  Create instance of a hardware serial port object
     *  @param  uartPort UART number [0 | 1]
     *  @note   A global instance of UART 0 is already defined as Serial
     */
	HardwareSerial(int uartPort) : uartNr(uartPort)
	{
	}

	~HardwareSerial();

	void setPort(int uartPort)
	{
		end();
		uartNr = uartPort;
	}

	/** @brief  Initialise the serial port
     *  @param  baud BAUD rate of the serial port (Default: 9600)
     */
	void begin(uint32_t baud = 9600)
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
	void begin(uint32_t baud, SerialConfig config)
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
	void begin(uint32_t baud, SerialConfig config, SerialMode mode)
	{
		begin(baud, config, mode, 1);
	}

	void begin(uint32_t baud, SerialConfig config, SerialMode mode, uint8_t txPin);

	/**
	 * @brief De-inits the current UART if it is already used
	 */
	void end();

	/**
	 * @brief Sets receiving buffer size
	 * @param size_t requested size
	 * @retval size_t actual size
	 */
	size_t setRxBufferSize(size_t size);

	/**
	 * @brief Sets transmit buffer size
	 * @param size_t requested size
	 * @retval size_t actual size
	 */
	size_t setTxBufferSize(size_t size);

	/**
	 * @brief Governs write behaviour when UART transmit buffers are full
	 * If false, writes will return short count; applications can use the txComplete callback to send more data.
	 * If true, writes will wait for more buffer space so that all requested data is written
	 */
	void setTxWait(bool wait)
	{
		bitWrite(options, UART_OPT_TXWAIT, wait);
		uart_set_options(uart, options);
	}

	/**
	 * @brief Toggle between use of GPIO13/GPIO15 or GPIO3/GPIO(1/2) as RX and TX
	 * @note UART0 uses pins GPIO1 (TX) and GPIO3 (RX). It may be swapped to GPIO15 (TX) and GPIO13 (RX) by calling .swap() after .begin. C
	 * @note Calling swap again maps UART0 back to GPIO1 and GPIO3.
	 */
	void swap()
	{
		swap(1);
	}

	/**
	 * @brief Toggle between use of GPIO13/GPIO15 or GPIO3/GPIO(1/2) as RX and TX
	 * @param uint8_t Pin number.
	 */
	void swap(uint8_t tx_pin)
	{
		uart_swap(uart, tx_pin);
	}

	/**
	 * @brief Toggle between use of GPIO1 and GPIO2 as TX on UART 0.
	 * @note: UART 1 can't be used if GPIO2 is used with UART 0!
	 * @note: If UART1 is not used and UART0 is not swapped - TX for UART0 can be mapped to GPIO2 by calling .setTx(2) after
	 * 		  .begin or directly with .begin(baud, config, mode, 2).
	 *
	 */
	void setTx(uint8_t tx_pin)
	{
		uart_set_tx(uart, tx_pin);
	}

	/**
	 * @brief Sets the transmission and receiving PINS
	 * @param uint8_t tx Transmission pin number
	 * @param uint8_t rx Receiving pin number
	 * @note UART 0 possible options are (1, 3), (2, 3) or (15, 13)
	 * @note UART 1 allows only TX on 2 if UART 0 is not (2, 3)
	 */
	void pins(uint8_t tx, uint8_t rx)
	{
		uart_set_pins(uart, tx, rx);
	}

	/** @brief  Get quantity characters available from serial input
     *  @retval int Quantity of characters in receive buffer
     */
	int available() override
	{
		return (int)uart_rx_available(uart);
	}

	/** @brief  Read a character from serial port
     *  @retval int Character read from serial port or -1 if buffer empty
     *  @note   The character is removed from the serial port input buffer
    */
	int read() override
	{
		return uart_read_char(uart);
	}

	/** @brief  Read a block of characters from serial port
	 *  @param  buf Pointer to buffer to hold received data
	 *  @param  max_len Maximum quantity of characters to read
	 *  @retval size_t Quantity of characters read
	 *  @note Although this shares the same name as the method in IDataSourceStream,
	 *  behaviour is different because in effect the 'seek' position is changed by this call.
	 */
	uint16_t readMemoryBlock(char* buf, int max_len) override
	{
		return uart_read(uart, buf, max_len);
	}

	bool seek(int len) override
	{
		return false;
	}

	bool isFinished() override
	{
		return false;
	}

	/** @brief  Read a character from serial port without removing from input buffer
     *  @retval int Character read from serial port or -1 if buffer empty
     *  @note   The character remains in serial port input buffer
     */
	int peek() override
	{
		return uart_peek_char(uart);
	}

	/** @brief  Clear the serial port transmit/receive buffers
 	 *  @note   All un-read buffered data is removed
	 */
	void clear()
	{
		uart_flush(uart);
	}

	/** @brief Flush all pending data to the serial port
	 *  @note Not to be confused with uart_flush() which is different. See clear() method.
	 */
	void flush()
	{
		uart_wait_tx_empty(uart);
	}

	using Stream::write;

	/** @brief  write multiple characters to serial port
	 *  @param buffer data to write
	 *  @param size number of characters to write
	 *  @retval size_t Quantity of characters written, may be less than size
	 */
	size_t write(const uint8_t* buffer, size_t size) override
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
	bool setCallback(StreamDataReceivedDelegate dataReceivedDelegate)
	{
		return onDataReceived(dataReceivedDelegate);
	}

	/** @brief  Set handler for received data
	 *  @param  reqCallback Function to handle received data
	 *  @retval bool Returns true if the callback was set correctly
	 */
	bool onDataReceived(StreamDataReceivedDelegate dataReceivedDelegate)
	{
		this->HWSDelegate = dataReceivedDelegate;
		return updateUartCallback();
	}

	/** @brief  Set handler for received data
	 *  @param  reqCallback Function to handle received data
	 *  @retval bool Returns true if the callback was set correctly
	 */
	bool onTransmitComplete(TransmitCompleteDelegate transmitCompleteDelegate)
	{
		this->transmitComplete = transmitCompleteDelegate;
		return updateUartCallback();
	}

	/**
	 * @brief  Set callback ISR for received data
	 * @param  reqCallback Function to handle received data
	 * @note callback is invoked directly from serial ISR and bypasses any registered delgates
	 */
	__forceinline void setUartCallback(uart_callback_t callback, void* param = nullptr)
	{
		uart_set_callback(uart, callback, param);
	}

	/**
	 * @brief  Checks if the current UART can transmit(print) information
	 * @retval bool true if transmitting is allowed
	 */
	bool isTxEnabled()
	{
		return uart_tx_enabled(uart);
	}

	/**
	 * @brief  Checks if the current UART can receive information
	 * @retval bool true if receiving is allowed
	 */
	bool isRxEnabled()
	{
		return uart_rx_enabled(uart);
	}

	/**
	 * @brief Get the current baud rate
	 * @retval uint32_t baud rate
	 */
	uint32_t baudRate()
	{
		return uart_get_baudrate(uart);
	}

	/**
	 * @brief Attempt to set the requested baud rate
	 * @retval uint32_t the actual baud rate in force
	 * @note Return value may differ from requested baud rate due to clock division errors
	 */
	uint32_t setBaudRate(uint32_t baudrate)
	{
		return uart_set_baudrate(uart, baudrate);
	}

	/**
	 * @brief Operator that returns true if the uart structure is set
	 */
	operator bool() const
	{
		return uart != nullptr;
	}

	/**
	 * @brief Returns the location of the searched character
	 * @param char c - character to search for
	 * @retval int -1 if not found 0 or positive number otherwise
	 */
	int indexOf(char c) override
	{
		return uart_rx_find(uart, c);
	}

	/**
	 * @brief Returns a pointer to the internal uart object. Use with care.
	 * @retval pointer to uart_t
	 */
	uart_t* getUart()
	{
		return uart;
	}

private:
	int uartNr = -1;
	TransmitCompleteDelegate transmitComplete = nullptr; ///< Callback for transmit completion
	StreamDataReceivedDelegate HWSDelegate = nullptr;	///< Callback for received data
#if ENABLE_CMD_EXECUTOR
	CommandExecutor* commandExecutor = nullptr; ///< Callback for command execution (received data)
#endif
	uart_t* uart = nullptr;
	uart_options_t options = _BV(UART_OPT_TXWAIT);
	size_t txSize = DEFAULT_TX_BUFFER_SIZE;
	size_t rxSize = DEFAULT_RX_BUFFER_SIZE;

	/**
	 * @brief Serial interrupt handler, called by serial driver
	 * @param uart_t* pointer to UART object
	 * @param status UART status flags indicating cause(s) of interrupt
	 */
	static void IRAM_ATTR staticCallbackHandler(uart_t* uart, uint32_t status);
	void IRAM_ATTR callbackHandler(uint32_t status);

	/**
	 * @brief Called whenever one of the user callbacks change
	 * @retval true if uart callback is active
	 */
	bool updateUartCallback();
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
#endif /* _SMING_CORE_HARDWARE_SERIAL_H_ */
