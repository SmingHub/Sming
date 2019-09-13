/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * NanoTime.cpp
 *
 * @author mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "NanoTime.h"

#include <WString.h>

namespace NanoTime
{
const char* unitToString(Unit unit)
{
	switch(unit) {
	case Nanoseconds:
		return "ns";
	case Microseconds:
		return "us";
	case Milliseconds:
		return "ms";
	case Seconds:
		return "s";
	case Minutes:
		return "m";
	case Hours:
		return "h";
	case Days:
		return "d";
	default:
		return "?s";
	}
}

const char* unitToLongString(Unit unit)
{
	switch(unit) {
	case Nanoseconds:
		return "nanoseconds";
	case Microseconds:
		return "microseconds";
	case Milliseconds:
		return "milliseconds";
	case Seconds:
		return "seconds";
	case Minutes:
		return "minutes";
	case Hours:
		return "hours";
	case Days:
		return "days";
	default:
		return "?s";
	}
}

String Frequency::toString() const
{
	auto freq = frequency;
	unsigned div = 0;
	while(freq % 1000 == 0) {
		freq /= 1000;
		++div;
	}
	String s(freq);
	if(div == 1) {
		s += 'K';
	} else if(div == 2) {
		s += 'M';
	} else if(div == 3) {
		s += 'G';
	}
	s += "Hz";
	return s;
}

template <unsigned BufSize> class FormatBuffer
{
public:
	FormatBuffer()
	{
		buffer[0] = '\0';
	}

	void add(unsigned value, unsigned digits)
	{
		pos += strlen(ultoa_wp(value, &buffer[pos], 10, digits, '0'));
	}

	void add(char c)
	{
		buffer[pos++] = c;
	}

	operator String() const
	{
		return String(buffer, pos);
	}

private:
	char buffer[BufSize];
	unsigned pos = 0;
};

String TimeValue::toString() const
{
	if(overflow) {
		return "(OVF)";
	}

	FormatBuffer<64> buf;

	if(days != 0) {
		buf.add(days, 0);
		buf.add('d');
		buf.add(' ');
	}

	buf.add(hours, 2);
	buf.add(':');
	buf.add(minutes, 2);
	buf.add(':');
	buf.add(seconds, 2);

	if(unit < NanoTime::Seconds) {
		buf.add('.');
		buf.add(milliseconds, 3);
		if(microseconds != 0 || nanoseconds != 0) {
			buf.add(microseconds, 3);
			if(nanoseconds != 0) {
				buf.add(nanoseconds, 3);
			}
		}
	}

	return buf;
}

} // namespace NanoTime
