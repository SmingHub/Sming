/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 * Manages GDB packet encoding. See here for details:
 *
 * https://sourceware.org/gdb/current/onlinedocs/gdb/Remote-Protocol.html
 *
 ****/

#pragma once

#include "gdbuart.h"

class GdbPacket
{
public:
	// Automatically start sending a packet
	__forceinline GdbPacket()
	{
		start();
	}

	// Finish sending a packet
	__forceinline ~GdbPacket()
	{
		end();
	}

	// Send a char as part of a packet
	void writeChar(char c);

	/** @brief Send a character, escaping if required */
	void writeCharEscaped(char c);

	/** @brief Send a block of data, escaping as required */
	void writeEscaped(const void* data, unsigned length);

	/** @brief Output 8-bit value */
	void writeHexByte(uint8_t value);

	/** @brief Output 16-bit value */
	void writeHexWord16(uint16_t value);

	/** @brief Output 32-bit value */
	void writeHexWord32(uint32_t value);

	/**
	 * @brief Output a block of data, hex-encoded
	 * @param src
	 * @param size
	 */
	void writeHexBlock(const void* src, size_t size);

	/** @brief Output 'xxxxxxxx' to indicate undefined register value */
	void writeX32();

	/** @brief Output block of data exactly as given without escaping */
	void write(const void* data, unsigned length);

	/** @brief Output a null-terminated string exactly as given without escaping */
	void writeStr(const char* str);

	/** @brief Output a string reference in addr/len format */
	void writeStrRef(const char* str);

	size_t getLength()
	{
		return packetLength + 3; // Include # and checksum digits
	}

	/**
	 * @brief Decode a variable-length hex value, MSB first
	 * @param data will get positioned on the end of the hex string, as far as the routine has read into it
	 * @retval uint32_t decoded value
	 */
	static uint32_t readHexValue(const char*& data);

	/** @brief Encode a value as hex characters, LSB first
	 *  @param dst Location for output, will NOT be nul-terminated
	 *  @param src Data bytes to encode
	 *  @param size Size of source data in bytes
	 *  @note destination buffer must have enough space for (size * 2) chars
	 */
	static void encodeHexBlock(char* dst, const void* src, size_t size);

	/** @brief Decode hex-encoded data block
	 *  @param dst buffer for decoded hex bytes (may be same as src)
	 *  @param src source data, on return points to first character after hex data
	 *  @retval size_t number of decoded bytes
	 *  @note Output is always smaller than input so safe to overwrite src
	 */
	static size_t decodeHexBlock(void* dst, const char*& src);

private:
	// Send the start of a packet; reset checksum calculation.
	void start();

	// Finish sending a packet.
	void end();

private:
	uint8_t checksum = 0; ///< Running checksum of the output packet
	unsigned packetLength = 0;
};
