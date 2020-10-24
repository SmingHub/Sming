/* $Id: Stream.cpp 1151 2014-08-01 21:13:05Z  $
||
|| @author         David A. Mellis
|| @url            http://wiring.org.co/
|| @contribution   parsing functions based on TextFinder library by Michael Margolis
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

#include "Stream.h"

#include <Platform/Timers.h>

#define PARSE_TIMEOUT 1000 // default number of milli-seconds to wait
#define NO_SKIP_CHAR 1	 // a magic char not found in a valid ASCII numeric field

int Stream::timedRead()
{
	OneShotFastMs timer(receiveTimeout);

	int c;
	while((c = read()) < 0 && !timer.expired()) {
	}

	return c;
}

int Stream::timedPeek()
{
	OneShotFastMs timer(receiveTimeout);

	int c;
	while((c = peek()) < 0 && !timer.expired()) {
	}

	return c;
}

int Stream::peekNextDigit()
{
	int c;
	while((c = timedPeek()) >= 0 && c != '-' && c < '0' && c > '9') {
		read(); // discard non-numeric
	}
	return c;
}

bool Stream::findUntil(const char* target, const char* terminator)
{
	return findUntil(target, strlen(target), terminator, strlen(terminator));
}

bool Stream::findUntil(const char* target, size_t targetLen, const char* terminator, size_t termLen)
{
	if(target == nullptr || *target == '\0') {
		return true; // target is empty
	}

	size_t index = 0;
	size_t termIndex = 0;
	int c;
	while((c = timedRead()) > 0) {
		if(c != target[index]) {
			index = 0; // reset index, char doesn't match
		}

		if(c == target[index]) {
			++index;
			if(index >= targetLen) {
				// All chars matched
				return true;
			}
		}

		if(termLen > 0 && c == terminator[termIndex]) {
			if(++termIndex >= termLen) {
				return false; // terminate string found before target string
			}
		} else {
			termIndex = 0;
		}
	}

	// No match
	return false;
}

long Stream::parseInt()
{
	return parseInt(NO_SKIP_CHAR); // terminate on first non-digit character (or timeout)
}

// as above but a given skipChar is ignored
// this allows format characters (typically commas) in values to be ignored
long Stream::parseInt(char skipChar)
{
	bool isNegative = false;
	long value = 0;
	int c;

	c = peekNextDigit();
	// ignore non numeric leading characters
	if(c < 0) {
		return 0; // timeout
	}

	do {
		if(c == skipChar) {
			// ignore this charactor
		} else if(c == '-') {
			isNegative = true;
		} else if(c >= '0' && c <= '9') {
			// Digit
			value = value * 10 + c - '0';
		}
		read(); // consume the character
		c = timedPeek();
	} while((c >= '0' && c <= '9') || c == skipChar);

	return isNegative ? -value : value;
}

// as parseInt but returns a floating point value
float Stream::parseFloat()
{
	return parseFloat(NO_SKIP_CHAR);
}

// as above but the given skipChar is ignored
// this allows format characters (typically commas) in values to be ignored
float Stream::parseFloat(char skipChar)
{
	bool isNegative = false;
	bool isFraction = false;
	long value = 0;
	char c;
	float fraction = 1.0;

	c = peekNextDigit();
	// ignore non numeric leading characters
	if(c < 0) {
		return 0; // timeout
	}

	do {
		if(c == skipChar) {
			// ignore
		} else if(c == '-') {
			isNegative = true;
		} else if(c == '.') {
			isFraction = true;
		} else if(c >= '0' && c <= '9') {
			// digit
			value = value * 10 + c - '0';
			if(isFraction) {
				fraction *= 0.1;
			}
		}
		read(); // consume the character
		c = timedPeek();
	} while((c >= '0' && c <= '9') || c == '.' || c == skipChar);

	if(isNegative) {
		value = -value;
	}

	return isFraction ? value * fraction : value;
}

size_t Stream::readBytes(char* buffer, size_t length)
{
	size_t count = 0;
	int c;
	while(count < length && (c = timedRead()) >= 0) {
		*buffer++ = char(c);
		count++;
	}

	return count;
}

size_t Stream::readBytesUntil(char terminator, char* buffer, size_t length)
{
	if(buffer == nullptr || length < 1) {
		return 0;
	}

	size_t index = 0;
	int c;
	while(index < length && (c = timedRead()) >= 0 && c != terminator) {
		*buffer++ = char(c);
		index++;
	}

	return index; // length, excluding null terminator
}

String Stream::readString(size_t maxLen)
{
	String s;
	int c;
	while(s.length() < maxLen && (c = timedRead()) >= 0) {
		s += char(c);
	}
	return s;
}

String Stream::readStringUntil(char terminator)
{
	String s;
	int c;
	while((c = timedRead()) >= 0 && c != terminator) {
		s += char(c);
	}
	return s;
}
