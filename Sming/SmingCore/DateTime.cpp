/*
  DateTime.cpp - Arduino Date and Time library
  Copyright (c) Michael Margolis.  All right reserved.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "DateTime.h"
#include "Data/CStringArray.h"
#include <stdlib.h>

#define LEAP_YEAR(year) ((year % 4) == 0)

/*
 * Used to parse HTTP date strings - see fromHttpDate()
 */
static DEFINE_FSTR(flashMonthNames, LOCALE_MONTH_NAMES);
static DEFINE_FSTR(flashDayNames, LOCALE_DAY_NAMES);

/** @brief Get the number of days in a month, taking leap years into account
 *  @param month 0=jan
 *  @param year
 *  @retval uint8_t number of days in the month
 */
static uint8_t getMonthDays(uint8_t month, uint8_t year)
{
	static const uint8_t monthDays[] PROGMEM = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	return (month == 1 && LEAP_YEAR(year)) ? 29 : pgm_read_byte(&monthDays[month]);
}

//******************************************************************************
//* DateTime Public Methods
//******************************************************************************

DateTime::DateTime(time_t time)
{
	setTime(time);
}

void DateTime::setTime(time_t time)
{
	fromUnixTime(time, &Second, &Minute, &Hour, &Day, &DayofWeek, &Month, &Year);
	Milliseconds = 0;
	calcDayOfYear();
}

void DateTime::setTime(int8_t sec, int8_t min, int8_t hour, int8_t day, int8_t month, int16_t year)
{
	Second = sec;
	Minute = min;
	Hour = hour;
	Day = day;
	Month = month;
	Year = year;
	Milliseconds = 0;
	calcDayOfYear();
}

bool DateTime::isNull()
{
	return Second == 0 && Minute == 0 && Hour == 0 && Day == 0 && Month == 0 && Year == 0 && DayofWeek == 0 &&
		   Milliseconds == 0;
}

bool DateTime::fromHttpDate(const String& httpDate)
{
	if(httpDate.length() < 29)
		return false;

	auto ptr = httpDate.c_str();

	// Parse and return a decimal number and update ptr to the first non-numeric character after it
	auto parseNumber = [&ptr]() { return strtol(ptr, const_cast<char**>(&ptr), 10); };

	char dayName[] = {ptr[0], ptr[1], ptr[2], '\0'};
	for(DayofWeek = 0; DayofWeek < 7; ++DayofWeek) {
		if(strncmp(CStringArray(flashDayNames)[DayofWeek], dayName, 3) == 0)
			break;
	}
	if(DayofWeek > 6)
		return false; // Invalid day of week
	ptr += 5;

	Day = parseNumber();
	if(*ptr == '\0')
		return false;

	// Decode the month name
	ptr++;
	char monthName[] = {ptr[0], ptr[1], ptr[2], '\0'};
	ptr += 4; // Skip space as well as month

	// Search is case insensitive
	for(Month = 0; Month < 12; ++Month) {
		if(strncmp(CStringArray(flashMonthNames)[Month], monthName, 3) == 0)
			break;
	}
	if(Month > 11)
		return false; // Invalid month

	Year = parseNumber();
	if(*ptr == '\0')
		return false;

	if(Year < 70)
		Year += 2000;
	else if(Year < 100)
		Year += 1900;

	Hour = parseNumber();
	if(*ptr != ':')
		return false;

	ptr++;
	Minute = parseNumber();
	if(*ptr != ':')
		return false;

	ptr++;
	Second = parseNumber();
	Milliseconds = 0;
	calcDayOfYear();

	return true;
}

time_t DateTime::toUnixTime()
{
	return toUnixTime(Second + (Milliseconds / 1000), Minute, Hour, Day, Month, Year);
}

String DateTime::toShortDateString()
{
	return format(_F("%d.%m.%Y"));
}

String DateTime::toShortTimeString(bool includeSeconds /* = false*/)
{
	if(includeSeconds)
		return format(_F("%T"));
	else
		return format(_F("%r"));
}

String DateTime::toFullDateTimeString()
{
	return format(_F("%x %T"));
}

String DateTime::toISO8601()
{
	return format(_F("%FT%TZ"));
}

String DateTime::toHTTPDate()
{
	return format(_F("%a, %d %b %Y %T GMT"));
}

void DateTime::addMilliseconds(long add)
{
	int ms = Milliseconds;
	ms += add;
	time_t cur = toUnixTime();
	cur += (ms / 1000);
	ms = ms % 1000;
	setTime(cur);
	Milliseconds = ms;
}

void DateTime::fromUnixTime(time_t timep, int8_t* psec, int8_t* pmin, int8_t* phour, int8_t* pday, int8_t* pwday,
							int8_t* pmonth, int16_t* pyear)
{
	// convert the given time_t to time components
	// this is a more compact version of the C library localtime function

	unsigned long epoch = timep;
	if(psec)
		*psec = epoch % 60;
	epoch /= 60; // now it is minutes
	if(pmin)
		*pmin = epoch % 60;
	epoch /= 60; // now it is hours
	if(phour)
		*phour = epoch % 24;
	epoch /= 24; // now it is days
	if(pwday)
		*pwday = (epoch + 4) % 7;

	unsigned year = 70;
	unsigned long days = 0;
	while((days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch) {
		year++;
	}
	if(pyear)
		*pyear = year + 1900; // *pyear is returned as years from 1900

	days -= LEAP_YEAR(year) ? 366 : 365;
	epoch -= days; // now it is days in this year, starting at 0
	//*pdayofyear=epoch;  // days since jan 1 this year

	int8_t month;
	for(month = 0; month < 12; month++) {
		uint8_t monthDays = getMonthDays(month, year);
		if(epoch >= monthDays) {
			epoch -= monthDays;
		} else {
			break;
		}
	}

	if(pmonth)
		*pmonth = month; // jan is month 0
	if(pday)
		*pday = epoch + 1; // day of month
}

time_t DateTime::toUnixTime(int8_t sec, int8_t min, int8_t hour, int8_t day, int8_t month, int16_t year)
{
	// converts time components to time_t
	// note year argument is full four digit year (or digits since 2000), i.e.1975, (year 8 is 2008)

	if(year < 69)
		year += 2000;
	// seconds from 1970 till 1 jan 00:00:00 this year
	time_t seconds = (year - 1970) * (SECS_PER_DAY * 365);

	// add extra days for leap years
	for(int i = 1970; i < year; i++) {
		if(LEAP_YEAR(i)) {
			seconds += SECS_PER_DAY;
		}
	}
	// add days for this year
	for(int i = 0; i < month; i++) {
		seconds += SECS_PER_DAY * getMonthDays(i, year);
	}

	seconds += (day - 1) * SECS_PER_DAY;
	seconds += hour * SECS_PER_HOUR;
	seconds += min * SECS_PER_MIN;
	seconds += sec;
	return seconds;
}

String DateTime::format(String sFormat)
{
	String sReturn;
	char buf[64]; //64 characters should be sufficient for most locale

	for(unsigned int pos = 0; pos < sFormat.length(); ++pos) {
		if(sFormat[pos] == '%') {
			if(pos < sFormat.length() - 1) //Ignore % as last character
			{
				buf[0] = '\0'; //Empty string in case no format character matches
				switch(sFormat[pos + 1]) {
				//Year (not implemented: EY, Oy, Ey, EC, G, g)
				case 'Y': //Full year as a decimal number, e.g. 2018
					m_snprintf(buf, sizeof(buf), _F("%04d"), Year);
					break;
				case 'y': //Year, last 2 digits as a decimal number [00..99]
					m_snprintf(buf, sizeof(buf), _F("%02d"), Year % 100);
					break;
				case 'C': //Year, first 2 digits as a decimal number [00..99]
					m_snprintf(buf, sizeof(buf), _F("%02d"), Year / 100);
					break;
				//Month (not implemented: Om)
				case 'b': //Abbreviated month name, e.g. Oct (always English)
				case 'h': //Synonym of b
					m_snprintf(buf, 4, _F("%s"), CStringArray(flashMonthNames)[Month]);
					break;
				case 'B': //Full month name, e.g. October (always English)
					m_snprintf(buf, sizeof(buf), _F("%s"), CStringArray(flashMonthNames)[Month]);
					break;
				case 'm': //Month as a decimal number [01..12]
					m_snprintf(buf, sizeof(buf), _F("%02d"), Month + 1);
					break;
				//Week (not implemented: OU, OW, V, OV)
				case 'U': //Week of the year as a decimal number (Sunday is the first day of the week) [00..53]
				{
					int16_t sunday = DayofYear - DayofWeek;
					int8_t firstSunday = sunday % 7;
					if(firstSunday < 0)
						firstSunday += 7;
					int16_t weekNumber = (sunday + 7 - firstSunday) / 7;
					m_snprintf(buf, sizeof(buf), _F("%03d"), weekNumber);
					break;
				}
				case 'V': //ISO 8601 week number (01-53)
					//!@todo Implement ISO 8601 week number
					break;
				case 'W': //Week of the year as a decimal number (Monday is the first day of the week) [00..53]
					//!@todo Implement week number (Monday as first day of week)
					break;
				case 'x': //Locale preferred date format
					m_snprintf(buf, sizeof(buf), _F("%s"), format(LOCALE_DATE).c_str());
					break;
				case 'X': //Locale preferred time format
					m_snprintf(buf, sizeof(buf), _F("%s"), format(LOCALE_TIME).c_str());
					break;
				// Day of year/month (Not implemented: Od, Oe)
				case 'j': //Day of the year as a decimal number [001..366]
				{
					m_snprintf(buf, sizeof(buf), _F("%03d"), DayofYear);
					break;
				}
				case 'd': //Day of the month as a decimal number [01..31]
					m_snprintf(buf, sizeof(buf), _F("%02d"), Day);
					break;
				case 'e': //Day of the month as a decimal number [ 1,31]
					m_snprintf(buf, sizeof(buf), _F("% 2d"), Day);
					break;
				// Day of week (Not implemented: Ow, Ou)
				case 'w': //Weekday as a decimal number with Sunday as 0 [0..6]
					m_snprintf(buf, sizeof(buf), _F("%d"), DayofWeek);
					break;
				case 'a': //Abbreviated weekday name, e.g. Fri
					m_snprintf(buf, 4, _F("%s"), CStringArray(flashDayNames)[DayofWeek]);
					break;
				case 'A': //Full weekday name, e.g. Friday
					m_snprintf(buf, sizeof(buf), _F("%s"), (CStringArray(flashDayNames)[DayofWeek]));
					break;
				case 'u': //Weekday as a decimal number, where Monday is 1 (ISO 8601 format) [1..7]
					m_snprintf(buf, sizeof(buf), _F("%d"), (DayofWeek == 0) ? 7 : DayofWeek);
					break;
				//Time (not implemented: OH, OI, OM, OS)
				case 'H': //Hour as a decimal number, 24 hour clock [00..23]
					m_snprintf(buf, sizeof(buf), _F("%02d"), Hour);
					break;
				case 'I': //Hour as a decimal number, 12 hour clock [0..12]
					m_snprintf(buf, sizeof(buf), _F("%02d"), Hour ? ((Hour > 12) ? Hour - 12 : Hour) : 12);
					break;
				case 'M': //Minute as a decimal number [00..59]
					m_snprintf(buf, sizeof(buf), _F("%02d"), Minute);
					break;
				case 'S': //Second as a decimal number [00..61]
					m_snprintf(buf, sizeof(buf), _F("%02d"), Second);
					break;
				// Other (not implemented: Ec, Ex, EX, z, Z)
				case 'c': //Locale preferred date and time format, e.g. Tue Dec 11 08:48:32 2018
					m_snprintf(buf, sizeof(buf), _F("%s"), format(LOCALE_DATE_TIME).c_str());
					break;
				case 'D': //US date (MM/DD/YY)
					m_snprintf(buf, sizeof(buf), _F("%s"), format("%m/%d/%y").c_str());
					break;
				case 'F': //ISO 8601 date format (YYYY-mm-dd)
					m_snprintf(buf, sizeof(buf), _F("%s"), format("%Y-%m-%d").c_str());
					break;
				case 'r': //12-hour clock time (hh:MM:SS AM)
					m_snprintf(buf, sizeof(buf), _F("%s"), format("%I:%M:%S %p").c_str());
					break;
				case 'R': //Short time (HH:MM)
					m_snprintf(buf, sizeof(buf), _F("%02d:%02d"), Hour, Minute);
					break;
				case 'T': //ISO 8601 time format (HH:MM:SS)
					m_snprintf(buf, sizeof(buf), _F("%s"), format("%H:%M:%S").c_str());
					break;
				case 'p': //Meridiem [AM,PM]
					m_snprintf(buf, sizeof(buf), _F("%s"), (Hour < 12) ? "AM" : "PM");
					break;
				case '%': //Literal percent (%). The full conversion specification must be %%
					m_snprintf(buf, sizeof(buf), _F("%s"), "%");
					break;
				case 'n': //Newline character (\n)
					m_snprintf(buf, sizeof(buf), _F("%s"), "\n");
					break;
				case 't': //Horizontal tab (\t)
					m_snprintf(buf, sizeof(buf), _F("%s"), "\t");
					break;
				default: //Silently ignore % and process next character
					--pos;
				}
				sReturn += buf;
				++pos; //Skip format charachter
			}
		} else {
			//Normal character
			sReturn += sFormat[pos];
		}
	}
	return sReturn;
}

void DateTime::calcDayOfYear()
{
	DayofYear = 0;
	for(unsigned int i = 0; i < Month; ++i) {
		switch(i) {
		case 8:  //Sep
		case 3:  //Apr
		case 5:  //Jun
		case 10: //Nov
			DayofYear += 30;
			break;
		case 1: //Feb
			DayofYear += LEAP_YEAR(Year) ? 29 : 28;
			break;
		default:
			DayofYear += 31;
		}
	}
	DayofYear += Day;
}
