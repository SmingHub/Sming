/**	Basic_DateTime sample application
	Author: Brian Walton (brian@riban.co.uk)
	Date: 2018-12-10
	Provides serial interface, accepting Unix timestamp
	Prints each type of DateTime::format option
*/
#include <SmingCore.h>
#include <Data/Buffer/LineBuffer.h>

namespace
{
DEFINE_FSTR_LOCAL(commandPrompt, "Enter Unix timestamp: ");

void showTime(const DateTime& dt)
{
	auto printFormat = [&dt](const String& fmt, const String& msg) -> void {
		Serial << fmt << ' ' << msg << ": " << dt.format(fmt) << endl;
	};

	// Non-time
	printFormat("%%", F("Percent sign"));
	printFormat("%n", F("New-line character"));
	printFormat("|<", F("Horizontal-tab character: >|"));
	// Year
	printFormat("%Y", F("Full year (YYYY)"));
	printFormat("%C", F("Year, first two digits (00-99)"));
	printFormat("%y", F("Year, last two digits (00-99)"));
	// Month
	printFormat("%B", F("Full month name (e.g. June)"));
	printFormat("%b", F("Abbreviated month name (e.g. Jun)"));
	printFormat("%h", F("Abbreviated month name (e.g. Jun)"));
	printFormat("%m", F("Month as a decimal number (01-12)"));
	// Week
	printFormat("%U", F("Week number with the first Sunday as the first day of week one (00-53)"));
	printFormat("%W", F("Week number with the first Monday as the first day of week one (00-53)"));
	printFormat("%V", F("ISO 8601 week number (01-53)"));
	// Day
	printFormat("%j", F("Day of the year (001-366)"));
	printFormat("%d", F("Day of the month, zero-padded (01-31)"));
	printFormat("%e", F("Day of the month, space-padded ( 1-31)"));
	printFormat("%A", F("Full weekday name (e.g. Monday)"));
	printFormat("%a", F("Abbreviated weekday name (e.g. Mon)"));
	printFormat("%w", F("Weekday as a decimal number with Sunday as 0 (0-6)"));
	printFormat("%u", F("ISO 8601 weekday as number with Monday as 1 (1-7)"));
	// Hour
	printFormat("%p", F("Meridiem (AM|PM)"));
	printFormat("%H", F("Hour in 24h format (00-23)"));
	printFormat("%I", F("Hour in 12h format (01-12)"));
	// Minute
	printFormat("%M", F("Minute (00-59)"));
	// Second
	printFormat("%S", F("Second (00-61)"));
	// Formatted strings
	printFormat("%R", F("24-hour time (HH:MM)"));
	printFormat("%r", F("12-hour time (hh:MM:SS AM)"));
	printFormat("%c", F("Locale date and time"));
	printFormat("%D", F("US short date (MM/DD/YY)"));
	printFormat("%F", F("ISO 8601 date (YYYY-MM-DD)"));
	printFormat("%T", F("ISO 8601 time (HH:MM:SS)"));
	printFormat("%x", F("Locale date"));
	printFormat("%X", F("Locale time"));

	auto print = [](const String& tag, const String& value) { Serial << tag << ": " << value << endl; };

	// HTTP date
	print(F("toHTTPDate"), dt.toHTTPDate());
	DateTime dt2;
	dt2.fromHttpDate(dt.toHTTPDate());
	print(F("fromHTTPDate"), dt2.toHTTPDate());
	print(F("toFullDateTimeString"), dt.toFullDateTimeString());
	print(F("toISO8601"), dt.toISO8601());
	print(F("toShortDateString"), dt.toShortDateString());
	print(F("toShortTimeString"), dt.toShortTimeString());
}

void onRx(Stream& source, char arrivedChar, unsigned short availableCharsCount)
{
	static LineBuffer<32> buffer;

	using Action = LineBufferBase::Action;
	switch(buffer.process(source, Serial)) {
	case Action::submit: {
		if(!buffer) {
			break;
		}
		String s(buffer);
		buffer.clear();
		char* p;
		time_t timestamp = strtoll(s.c_str(), &p, 0);
		if(p == s.end()) {
			Serial << endl << _F("****Showing DateTime formatting options for Unix timestamp: ") << timestamp << endl;
			showTime(timestamp);
			break;
		}

		DateTime dt;
		if(dt.fromHttpDate(s)) {
			Serial << endl << _F("****Showing DateTime formatting options for Http time: ") << s << endl;
			showTime(dt);
			break;
		}

		if(dt.fromISO8601(s)) {
			Serial << endl << _F("****Showing DateTime formatting options for ISO8601 date/time: ") << s << endl;
			showTime(dt);
			break;
		}

		Serial << endl << _F("Please enter a valid numeric timestamp, HTTP or ISO8601 date/time string!") << endl;
		break;
	}

	case Action::clear:
		break;

	default:;
		return;
	}

	Serial.print(commandPrompt);
}

} // namespace

void init()
{
	Serial.begin(COM_SPEED_SERIAL);
	delay(2000);
	Serial.print(commandPrompt);
	Serial.onDataReceived(onRx);
}
