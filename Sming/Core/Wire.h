/*
  TwoWire.h - TWI/I2C library for Arduino & Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
  Modified December 2014 by Ivan Grokhotkov (ivan@esp8266.com) - esp8266 support
  Modified April 2015 by Hrsto Gochkov (ficeto@ficeto.com) - alternative esp8266 support
*/

#pragma once

#include <Stream.h>
#include <twi_arch.h>

class TwoWire : public Stream
{
public:
	static constexpr size_t BUFFER_LENGTH{32};

	enum Status {
		I2C_OK = 0,
		I2C_SCL_HELD_LOW = 1,
		I2C_SCL_HELD_LOW_AFTER_READ = 2,
		I2C_SDA_HELD_LOW = 3,
		I2C_SDA_HELD_LOW_AFTER_INIT = 4,
	};

	enum Error {
		I2C_ERR_SUCCESS = 0,
		I2C_ERR_ADDR_NACK = 2,
		I2C_ERR_DATA_NACK = 3,
		I2C_ERR_LINE_BUSY = 4,
	};

	using UserRequest = void (*)();
	using UserReceive = void (*)(int len);

	TwoWire() : Stream()
	{
	}

	/**
	 * @brief Initialise using selected pins
	 */
	void begin(uint8_t sda, uint8_t scl);

	/**
	 * @brief Switch to selected pins
	 * 
	 * If `begin()` has already been called then pins will change immediately,
	 * otherwise will be used by next call to `begin()`.
	 */
	void pins(uint8_t sda, uint8_t scl);

	/**
	 * @brief Initialise using current pin values
	 *
	 * Default pins depend on selected architecture.
	 */
	void begin();

	/**
	 * @brief End TwoWire operation
	 */
	void end();

	/**
	 * @brief Set approximate clock frequency
	 */
	void setClock(uint32_t frequency);

	/**
	 * @brief Set approximate time in microseconds that clocks may be stretched by
	 */
	void setClockStretchLimit(uint32_t limit);

	/**
	 * @brief Signal start of transaction
	 * @param address Device address
	 */
	void beginTransmission(uint8_t address);

	/**
	 * @brief Perform actual transaction with device
	 * @param sendStop By default, send a stop condition
	 */
	Error endTransmission(bool sendStop = true);

	/**
	 * @brief Perform a complete 'read' transaction
	 * @param address Device to talk to
	 * @param size Number of bytes to receive
	 * @param sendStop By default, send a stop condition
	 */
	uint8_t requestFrom(uint8_t address, uint8_t size, bool sendStop = true);

	/**
	 * @brief Query bus status
	 * @retval Status Indicates whether bus is available
	 */
	Status status();

	/* Stream methods */

	size_t write(uint8_t) override;
	size_t write(const uint8_t*, size_t) override;
	int available() override;
	int read() override;
	int peek() override;
	void flush() override;

	using Print::write;

	// Slave mode not currently implemented
	void onReceive([[maybe_unused]] UserReceive callback)
	{
		// userReceiveCallback = callback;
	}

	// Slave mode not currently implemented
	void onRequest([[maybe_unused]] UserRequest callback)
	{
		// userRequestCallback = callback;
	}

private:
	uint8_t twi_sda{DEFAULT_SDA_PIN};
	uint8_t twi_scl{DEFAULT_SCL_PIN};
	uint8_t twi_dcount{18};
	unsigned twi_clockStretchLimit{0};

	uint8_t rxBuffer[BUFFER_LENGTH]{};
	uint8_t rxBufferIndex{0};
	uint8_t rxBufferLength{0};

	uint8_t txAddress{0};
	uint8_t txBuffer[BUFFER_LENGTH]{};
	uint8_t txBufferIndex{0};
	uint8_t txBufferLength{0};

	bool transmitting{false};
	UserRequest userRequestCallback{nullptr};
	UserReceive userReceiveCallback{nullptr};

	void twi_delay(uint8_t v);
	bool twi_write_start();
	bool twi_write_stop();
	bool twi_write_bit(bool bit);
	bool twi_read_bit();
	bool twi_write_byte(uint8_t byte);
	uint8_t twi_read_byte(bool nack);
	Error twi_writeTo(uint8_t address, const uint8_t* buf, size_t len, bool sendStop);
	Error twi_readFrom(uint8_t address, uint8_t* buf, size_t len, bool sendStop);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TWOWIRE)
extern TwoWire Wire;
#endif
