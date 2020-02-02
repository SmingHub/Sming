/* $Id: Stream.h 1151 2011-06-06 21:13:05Z bhagman $
||
|| @author         Brett Hagman <bhagman@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
|| @contribution   David A. Mellis
||
|| @description
|| | Base class for streams.
|| |
|| | Wiring Common API
|| #
||
|| @notes
|| | Originally discussed here:
|| |
|| | http://code.google.com/p/arduino/issues/detail?id=60
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#pragma once

#include "Print.h"

/**
 * @brief Base Stream class
 * @ingroup wiring
 */
class Stream : public Print
{
public:
	virtual int available() = 0;
	virtual int peek() = 0;
	virtual int read() = 0;
	virtual void flush() = 0;

	Stream()
	{
	}

	// parsing methods

	/**
	 * @brief Set maximum milliseconds to wait for stream data, default is 1 second
	 */
	void setTimeout(unsigned long timeout);

	/**
	 * @brief Read data from the stream until the target string is found
	 * @param target String to watch for
	 * @retval bool true if target string is found, false if timed out (see setTimeout)
	 */
	bool find(const char* target);

	/**
	 * @brief Read data from the stream until the target string of given length is found
	 * @param target
	 * @param length
	 * @retval bool returns true if target string is found, false if timed out
	 */
	bool find(const char* target, size_t length);

	/**
	 * @brief As `find()` but search ends if the terminator string is found
	 */
	bool findUntil(const char* target, const char* terminator);

	/**
	 * @brief As `findUntil(const char*, const char*)` but search ends if the terminate string is found
	 */
	bool findUntil(const char* target, size_t targetLen, const char* terminate, size_t termLen);

	/**
     * @brief Returns the first valid (long) integer value from the current position.
     *
     * Initial characters that are not digits (or the minus sign) are skipped.
     * integer is terminated by the first character that is not a digit.
     */
	long parseInt();

	/**
     * @brief float version of parseInt
     */
	float parseFloat();

	/**
     * @brief Read chars from stream into buffer
     *
     * Terminates if length characters have been read or timeout (see setTimeout).
     * Returns the number of characters placed in the buffer (0 means no valid data found).
     *
     * @note Inherited classes may provide more efficient implementations without timeout.
     */
	virtual size_t readBytes(char* buffer, size_t length);

	/**
     * @brief As `readBytes()` with terminator character
     *
     * Terminates if length characters have been read, timeout, or if the terminator character detected.
     *
     * @retval size_t Number of characters placed in the buffer (0 means no valid data found)
     */
	size_t readBytesUntil(char terminator, char* buffer, size_t length);

	/**
	 * @brief Like `readBytes` but place content into a `String`
	 * @param maxLen Limit number of bytes read
	 * @retval String
	 */
	virtual String readString(size_t maxLen);

	String readStringUntil(char terminator);

	/*
	 * @brief Returns the location of the searched character
	 * @param c Character to search for
	 * @retval int -1 if not found 0 or positive number otherwise
	 */
	virtual int indexOf(char c)
	{
		return -1;
	}

protected:
	int timedRead();
	int timedPeek();

	/**
     *  @brief returns the next numeric digit in the stream or -1 if timeout
     */
	int peekNextDigit();

	/**
     * @brief Like regular `parseInt()` but the given skipChar is ignored
     *
     * This allows format characters (typically commas) in values to be ignored
     */
	long parseInt(char skipChar);

	/**
	 * @brief Like `parseInt(skipChar)` for float
	 */
	float parseFloat(char skipChar);

	uint16_t receiveTimeout = 1000; ///< number of milliseconds to wait for the next char before aborting timed read
};
