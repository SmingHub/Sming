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
 * Used to parse HTTP date strings - see parseHttpDate()
 */
static DEFINE_FSTR(flashMonthNames, "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec");

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
	convertFromUnixTime(time, &Second, &Minute, &Hour, &Day, &DayofWeek, &Month, &Year);
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

bool DateTime::parseHttpDate(const String& httpDate)
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
	Month = CStringArray(flashMonthNames).indexOf(monthName);
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
	return convertToUnixTime(Second + (Milliseconds / 1000), Minute, Hour, Day, Month, Year);
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
	m_snprintf(buf, sizeof(buf), _F("%02d-%02d-%02dT%02d:%02d:%02dZ"), Year, Month + 1, Day, Hour, Minute, Second);
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

void DateTime::convertFromUnixTime(time_t timep, int8_t* psec, int8_t* pmin, int8_t* phour, int8_t* pday, int8_t* pwday,
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

time_t DateTime::convertToUnixTime(int8_t sec, int8_t min, int8_t hour, int8_t day, int8_t month, int16_t year)
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
