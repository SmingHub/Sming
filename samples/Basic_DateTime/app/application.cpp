/**	Basic_DateTime sample application
	Author: Brian Walton (brian@riban.co.uk)
	Date: 2018-12-10
	Provides serial interface, accepting Unix timestamp
	Prints each type of DateTime::format option
*/
#include <user_config.h>
#include <SmingCore/SmingCore.h>

time_t g_timestamp = 0;
size_t g_tsLength = 0;

void showTime(time_t timestamp)
{
	DateTime dt(timestamp);
	//Non-time
	Serial.println(dt.format("%%%% Percent sign: %%"));
	Serial.println(dt.format("%%n New-line character: %n"));
	Serial.println(dt.format("%%t Horizontal-tab character: >|%t|<"));
	//Year
	Serial.println(dt.format("%%Y Full year (YYYY): %Y"));
	Serial.println(dt.format("%%C Year, first two digits (00-99)%: %C"));
	Serial.println(dt.format("%%y Year, last two digits (00-99): %y"));
	//Month
	Serial.println(dt.format("%%B Full month name (e.g. June): %B"));
	Serial.println(dt.format("%%b Abbreviated month name (e.g. Jun): %b"));
	Serial.println(dt.format("%%h Abbreviated month name (e.g. Jun): %h"));
	Serial.println(dt.format("%%m Month as a decimal number (01-12): %m"));
	//Week
	Serial.println(dt.format("%%U Week number with the first Sunday as the first day of week one (00-53): %U")); //NYI
	Serial.println(dt.format("%%W Week number with the first Monday as the first day of week one (00-53): %W")); //NYI
	Serial.println(dt.format("%%V ISO 8601 week number (01-53): %V")); //NYI
	//Day
	Serial.println(dt.format("%%j Day of the year (001-366): %j"));
	Serial.println(dt.format("%%d Day of the month, zero-padded (01-31)%: %d"));
	Serial.println(dt.format("%%e Day of the month, space-padded ( 1-31): %e"));
	Serial.println(dt.format("%%A Full weekday name (e.g. Monday): %A"));
	Serial.println(dt.format("%%a Abbreviated weekday name (e.g. Mon): %a"));
	Serial.println(dt.format("%%w Weekday as a decimal number with Sunday as 0 (0-6): %w"));
	Serial.println(dt.format("%%u ISO 8601 weekday as number with Monday as 1 (1-7): %u"));
	//Hour
	Serial.println(dt.format("%%p Meridiem (AM|PM): %p"));
	Serial.println(dt.format("%%H Hour in 24h format (00-23): %H"));
	Serial.println(dt.format("%%h Hour in 12h format (01-12): %I"));
	//Mnute
	Serial.println(dt.format("%%M Minute (00-59): %M"));
	//Second
	Serial.println(dt.format("%%S Second (00-61): %S"));
	//Formatted strings
	Serial.println(dt.format("%%R 24-hour time (HH:MM): %R"));
	Serial.println(dt.format("%%r 12-hour time (hh:MM:SS AM): %r"));
	Serial.println(dt.format("%%c Date and time (ddd mmm DD HH:MM:SS YYYY): %c"));
	Serial.println(dt.format("%%D Short date (MM/DD/YY): %D"));
	Serial.println(dt.format("%%F Short date (YYYY-MM-DD): %F"));
	Serial.println(dt.format("%%T ISO 8601 time format (HH:MM:SS): %T"));
	Serial.println(dt.format("%%x Local date (DD/MM/YYYY): %x"));
	Serial.println(dt.format("%%X Local time (HH:MM:SS): %X"));
	//HTTP date
	Serial.print("toHTTPDate: ");
	Serial.println(dt.toHTTPDate());
	DateTime dt2;
	dt2.fromHttpDate(dt.toHTTPDate());
	Serial.print("fromHTTPDate: ");
	Serial.println(dt2.toHTTPDate());
	Serial.print("toFullDateTimeString: ");
	Serial.println(dt.toFullDateTimeString());
	Serial.print("toISO8601: ");
	Serial.println(dt.toISO8601());
	Serial.print("toShortDateString: ");
	Serial.println(dt.toShortDateString());
	Serial.print("toShortTimeString: ");
	Serial.println(dt.toShortTimeString());

}

void onRx(Stream& source, char arrivedChar, unsigned short availableCharsCount)
{
	switch(arrivedChar)
	{
		case '\n':
			Serial.println();
			Serial.println();
			Serial.print("****Showing DateTime formating options for Unix timestamp: ");
			Serial.println(g_timestamp);
			showTime(g_timestamp);
			Serial.print("Enter Unix timestamp: ");
			g_timestamp = 0;
			g_tsLength = 0;
			break;
		case '0' ... '9':
			g_timestamp *= 10;
			g_timestamp += arrivedChar - '0';
			++g_tsLength;
			Serial.print(arrivedChar);
			break;
		case '\b':
			if(g_tsLength)
			{
				Serial.print('\b');
				Serial.print(" ");
				Serial.print('\b');
				--g_tsLength;
				g_timestamp /= 10;
			}
			break;
		case 27:
			g_timestamp = 0;
			g_tsLength = 0;
			Serial.print('\r');
			Serial.print("                                                     ");
			Serial.print('\r');
			Serial.print("Enter Unix timestamp: ");
	}
}

void init()
{
	Serial.begin(115200);
	delay(2000);
	Serial.print("Enter Unix timestamp: ");
	Serial.setCallback(onRx);
}
