/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * LineBuffer.h - support for buffering/editing a line of text
 *
 * author mikee47 <mike@sillyhouse.net> Feb 2019
 *
 ****/

#pragma once

#include <cstdint>
#include <cstring>
#include <WString.h>
#include <Data/Stream/ReadWriteStream.h>

/**
 * @brief Class to enable buffering of a single line of text, with simple editing
 * @note We define this as a template class for simplicity, no need for separate buffer memory management
 */
class LineBufferBase
{
public:
	LineBufferBase(char* buffer, uint16_t size) : buffer(buffer), size(size)
	{
	}

	/**
	 * @brief Returned from `processKey` method directing caller
	 */
	enum class Action {
		none,	  ///< Do nothing, ignore the key
		clear,	 ///< Line is cleared: typically perform a carriage return
		echo,	  ///< Key should be echoed
		backspace, ///< Perform backspace edit, e.g. output "\b \b"
		submit,	///< User hit return, process line and clear it
	};

	/**
	 * @brief Process all available data from `input`
	 * @param input Source of keystrokes
	 * @param output The output stream (e.g. Serial) for echoing
	 * @retval Action: none, clear or submit
	 */
	Action process(Stream& input, ReadWriteStream& output);

	/**
	 * @brief Process a keypress in a consistent manner for console editing
	 * @param key The keypress value
	 * @param output The output stream (e.g. Serial) for echoing, if required
	 * @retval Action
	 */
	Action processKey(char key, ReadWriteStream* output = nullptr);

	/**
	 * @brief Add a character to the buffer
	 * @retval char Character added to buffer, '\0' if ignored, '\n' if line is complete
	 */
	char addChar(char c);

	/**
	 * @brief Clear contents of buffer
	 */
	void clear()
	{
		length = 0;
	}

	explicit operator bool() const
	{
		return length != 0;
	}

	/**
	 * @brief Copy buffer contents into a String
	 * @retval String
	 */
	explicit operator String() const
	{
		return length ? String(buffer, length) : nullptr;
	}

	/**
	 * @brief Get the text, nul-terminated
	 */
	char* getBuffer()
	{
		return buffer;
	}

	/**
	 * @brief Get number of characters in the text line
	 */
	unsigned getLength() const
	{
		return length;
	}

	/**
	 * @brief Check for matching text at start of line, case-sensitive
	 * @param text
	 * @retval bool true if match found
	 */
	bool startsWith(const char* text) const;

	/**
	 * @brief Check for matching text anywhere in line, case-sensitive
	 * @param text
	 * @retval bool true if match found
	 */
	bool contains(const char* text) const;

	/**
	 * @brief Remove last character from buffer
	 * @retval bool true if character was removed, false if buffer is empty
	 */
	bool backspace();

	size_t printTo(Print& p) const
	{
		return p.write(buffer, length);
	}

private:
	char* buffer;
	uint16_t size;
	uint16_t length{0};		///< Number of characters stored
	char previousKey{'\0'}; ///< For processing CR/LF
};

/**
 * @brief Class to enable buffering of a single line of text, with simple editing
 * @note We define this as a template class for simplicity, no need for separate buffer memory management
 */
template <uint16_t BUFSIZE> class LineBuffer : public LineBufferBase
{
public:
	LineBuffer() : LineBufferBase(buffer, BUFSIZE)
	{
	}

private:
	char buffer[BUFSIZE]{};
};
