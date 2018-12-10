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
static DEFINE_FSTR(flashMonthNames, "January\0February\0March\0April\0May\0June\0July\0August\0September\0October\0November\0December");
static DEFINE_FSTR(flashDayNames, "Sunday\0Monday\0Tuesday\0Wednesday\0Thursday\0Friday\0Saturday");

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
}

bool DateTime::isNull()
{
	return Second == 0 && Minute == 0 && Hour == 0 && Day == 0 && Month == 0 && Year == 0 && DayofWeek == 0 &&
		   Milliseconds == 0;
}

bool DateTime::fromHttpDate(const String& httpDate)
{
	int first = httpDate.indexOf(',');
	if(first < 0 || httpDate.length() - first < 20)
		return false;

	first++; // Skip ','
	if(httpDate[first] == ' ')
		first++;

	auto ptr = httpDate.c_str() + first;

	// Parse and return a decimal number and update ptr to the first non-numeric character after it
	auto parseNumber = [&ptr]() { return strtol(ptr, const_cast<char**>(&ptr), 10); };

	Day = parseNumber();
	if(*ptr == '\0')
		return false;

	// Decode the month name
	ptr++;
	char monthName[] = {ptr[0], ptr[1], ptr[2], '\0'};
	ptr += 4; // Skip space as well as month
	if(*ptr == '\0')
		return false;

	// Search is case insensitive
	Month = CStringArray(flashMonthNames).indexOf(monthName); //!@todo This is broken by long month names
	if(Month < 0)
		return false; // Invalid month

	Year = parseNumber();
	if(*ptr == '\0')
		return false;

	if(Year < 69)
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

	return true;
}

time_t DateTime::toUnixTime()
{
	return toUnixTime(Second + (Milliseconds / 1000), Minute, Hour, Day, Month, Year);
}

String DateTime::toShortDateString()
{
	char buf[16];
	m_snprintf(buf, sizeof(buf), _F("%02d.%02d.%d"), Day, Month + 1, Year);
	return String(buf);
}

String DateTime::toShortTimeString(bool includeSeconds /* = false*/)
{
	char buf[16];
	if(includeSeconds)
		m_snprintf(buf, sizeof(buf), _F("%02d:%02d:%02d"), Hour, Minute, Second);
	else
		m_snprintf(buf, sizeof(buf), _F("%02d:%02d"), Hour, Minute);

	return String(buf);
}

String DateTime::toFullDateTimeString()
{
	return toShortDateString() + ' ' + toShortTimeString(true);
}

String DateTime::toISO8601()
{
	char buf[32];
	m_snprintf(buf, sizeof(buf), _F("%04d-%02d-%02dT%02d:%02d:%02dZ"), Year, Month + 1, Day, Hour, Minute, Second);
	return String(buf);
}

String DateTime::toHTTPDate()
{
	char buf[30];
	m_snprintf(buf, sizeof(buf), _F("%s, %02d %s %04d %02d:%02d:%02d GMT"), getShortName(CStringArray(flashDayNames)[DayofWeek]).c_str(), Day, getShortName(CStringArray(flashMonthNames)[Month]).c_str(), Year, Hour, Minute, Second);
	return String(buf);
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
	//!@todo Add localisation (maybe via pre-compiler conditions)
	//!@todo Rationalise comments (taken from two sources)
	String sReturn;
	char buf[64]; //!@todo Check size of buffer is optimal

    for(unsigned int pos = 0; pos < sFormat.length(); ++pos)
	{
		if(sFormat[pos] == '%')
		{
			if(pos < sFormat.length() - 1) //Ignore % as last character
			{
				buf[0] = '\0'; //Empty string in case no format character matches
				switch(sFormat[pos + 1])
				{
					//Year (not implemented: EY, Oy, Ey, EC, G, g)
					case 'Y': //writes year as a decimal number, e.g. 2017
						m_snprintf(buf, sizeof(buf), _F("%04d"), Year);
						break;
					case 'y': //writes last 2 digits of year as a decimal number (range [00,99])
						m_snprintf(buf, sizeof(buf), _F("%02d"), Year%100);
						break;
					case 'C': //writes first 2 digits of year as a decimal number (range [00,99])
						m_snprintf(buf, sizeof(buf), _F("%02d"), Year/100);
						break;
					//Month (not implemented: Om)
					case 'b': //writes abbreviated month name, e.g. Oct (always English)
					case 'h': //synonym of b
						m_snprintf(buf, sizeof(buf), _F("%s"), getShortName(CStringArray(flashMonthNames)[Month]).c_str());
						break;
					case 'B': //writes full month name, e.g. October (locale dependent)
						m_snprintf(buf, sizeof(buf), _F("%s"), CStringArray(flashMonthNames)[Month]);
						break;
					case 'm': //writes month as a decimal number (range [01,12])
						m_snprintf(buf, sizeof(buf), _F("%02d"), Month + 1);
						break;
					//Week (not implemented: OU, OW, V, OV)
					case 'U': //writes week of the year as a decimal number (Sunday is the first day of the week) (range [00,53])
						//!@todo Implement week of the year (from Sunday)
						m_snprintf(buf, sizeof(buf), _F("??"));
						break;
					case 'w': //Weekday as a decimal number with Sunday as 0 (0-6)
						m_snprintf(buf, sizeof(buf), _F("%d"), DayofWeek);
						break;
					case 'W': //writes week of the year as a decimal number (Monday is the first day of the week) (range [00,53])
						//!@todo Implement week of the year (from Monday)
						m_snprintf(buf, sizeof(buf), _F("??"));
						break;
					case 'x': //Date representation
						//!@todo Date locale
						m_snprintf(buf, sizeof(buf), _F("%s"), format("%d/%m/%Y").c_str());
						break;
					case 'X': //Time representation
						//!@todo Time locale
						m_snprintf(buf, sizeof(buf), _F("%s"), format("%H:%M:%S").c_str());
						break;
					// Day of year/month (Not implemented: Od, e, Oe)
					case 'j': //writes day of the year as a decimal number (range [001,366])
					{
						//!@todo Store day of year during timestamp parsing
						unsigned int nDays = 0;
						for(unsigned int i=0; i<Month;++i)
						{
							switch(i)
							{
								case 8: //Sep
								case 3: //Apr
								case 5: //Jun
								case 10: //Nov
									nDays += 30;
									break;
								case 1: //Feb
									nDays += LEAP_YEAR(Year)?29:28;
									break;
								default:
									nDays += 31;
							}
						}
						nDays += Day;
						m_snprintf(buf, sizeof(buf), _F("%03d"), nDays);
						break;
					}
					case 'd': //writes day of the month as a decimal number (range [01,31])
						m_snprintf(buf, sizeof(buf), _F("%02d"), Day);
						break;
					case 'e': //writes day of the month as a decimal number (range [1,31])
						m_snprintf(buf, sizeof(buf), _F("% 2d"), Day);
						break;
					// Day of week (Not implemented: Ow, Ou)
					case 'a': //writes abbreviated weekday name, e.g. Fri (locale dependent)
						m_snprintf(buf, sizeof(buf), _F("%s"), getShortName(CStringArray(flashDayNames)[DayofWeek]).c_str());
						break;
					case 'A': //writes full weekday name, e.g. Friday (locale dependent)
						m_snprintf(buf, sizeof(buf), _F("%s"), (CStringArray(flashDayNames)[DayofWeek]));
						break;
					case 'u': //writes weekday as a decimal number, where Monday is 1 (ISO 8601 format) (range [1-7])
						m_snprintf(buf, sizeof(buf), _F("%d"), (DayofWeek == 0)?7:DayofWeek);
						break;
					//Hour, minute, second (not implemented: OH, OI, OM, OS)
					case 'H': //writes hour as a decimal number, 24 hour clock (range [00-23])
						m_snprintf(buf, sizeof(buf), _F("%02d"), Hour);
						break;
					case 'I': //writes hour as a decimal number, 12 hour clock (range [1,12])
						m_snprintf(buf, sizeof(buf), _F("%02d"), Hour?((Hour > 12)?Hour-12:Hour):12);
						break;
					case 'M': //writes minute as a decimal number (range [00,59])
						m_snprintf(buf, sizeof(buf), _F("%02d"), Minute);
						break;
					case 'S': //writes second as a decimal number (range [00,60])
						m_snprintf(buf, sizeof(buf), _F("%02d"), Second);
						break;
					// Other (not implemented: Ec, Ex, EX, z, Z)
					case 'c': //writes standard date and time string, e.g. Sun Oct 17 04:41:13 2010 (English only)
						m_snprintf(buf, sizeof(buf), _F("%s"), format("%a %b %d %H:%M:%S %Y").c_str());
						break;
					case 'D': //equivalent to "%m/%d/%y"
						m_snprintf(buf, sizeof(buf), _F("%s"), format("%m/%d/%y").c_str());
						break;
					case 'F': //equivalent to "%Y-%m-%d" (the ISO 8601 date format)
						m_snprintf(buf, sizeof(buf), _F("%s"), format("%Y-%m-%d").c_str());
						break;
					case 'r': //12-hour clock time
						//!@todo Adjust for locale
						//!@todo 12 hour time looks wrong with leading zero
						m_snprintf(buf, sizeof(buf), _F("%s"), format("%I:%M:%S %p").c_str());
						break;
					case 'R': //equivalent to "%H:%M"
						m_snprintf(buf, sizeof(buf), _F("%02d:%02d"), Hour, Minute);
						break;
					case 'T': //equivalent to "%H:%M:%S" (the ISO 8601 time format)
						m_snprintf(buf, sizeof(buf), _F("%s"), format("%H:%M:%S").c_str());
						break;
					case 'p': //writes localized a.m. or p.m. (English only)
						m_snprintf(buf, sizeof(buf), _F("%s"), (Hour < 12)?"AM":"PM"); //!@todo Should am/pm, upper/lowercase, with/without delimitets?
						break;
					case '%': //writes literal %. The full conversion specification must be %%
						m_snprintf(buf, sizeof(buf), _F("%s"), "%");
						break;
					case 'n': //writes newline character
						m_snprintf(buf, sizeof(buf), _F("%s"), "\n");
						break;
					case 't': //writes horizontal tab character
						m_snprintf(buf, sizeof(buf), _F("%s"), "\t");
						break;
					default:
						m_snprintf(buf, sizeof(buf), _F("%s"), "??");
				}
				sReturn += buf;
				++pos; //Skip format charachter
			}
		}
		else
		{
			//Normal charachter
			sReturn += sFormat[pos];
		}
	}
	return sReturn;
}

String DateTime::getShortName(const char* longname)
{
	String sReturn(longname, 3);
	return sReturn;
}

