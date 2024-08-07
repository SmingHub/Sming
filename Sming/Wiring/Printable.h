/* $Id: Printable.h 1151 2011-06-06 21:13:05Z bhagman $
||
|| @author         Adrian McEwen
|| @url            http://wiring.org.co/
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
||
|| @description
|| | Abstract base class to allow "printing" of complex types through
|| | the Print class.
|| |
|| | Wiring Common API
|| #
||
|| @notes
|| | The Printable class provides a way for new classes to allow themselves
|| | to be printed.  By deriving from Printable and implementing the printTo
|| | method, it will then be possible for users to print out instances of this
|| | class by passing them into the usual Print::print and Print::println
|| | methods.
|| #
||
|| @example
|| | class T : public Printable {
|| |   public:
|| |     void printTo(Print &p) {
|| |       p.print("test");
|| |     }
|| | };
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#pragma once

#include <cstddef>

class Print;

class Printable
{
public:
	Printable() = default;

	Printable(const Printable&) = delete;
	Printable(Printable&&) = delete;
	Printable& operator=(const Printable&) = delete;
	Printable& operator=(Printable&&) = delete;

	virtual ~Printable() = default;

	virtual size_t printTo(Print& p) const = 0;
};
