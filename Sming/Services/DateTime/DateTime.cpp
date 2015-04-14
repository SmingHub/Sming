/*
  DateTime.cpp - Arduino Date and Time library
  Copyright (c) Michael Margolis.  All right reserved.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*/

#include "DateTime.h"
#include <Arduino.h>
#include <stdlib.h>
#include <stdio.h>

//extern unsigned long _time;

#define LEAP_YEAR(_year) ((_year%4)==0)
static  int8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};


//******************************************************************************
//* DateTime Public Methods
//******************************************************************************

DateTime::DateTime()
{
	Second = 0;
	Minute = 0;
	Hour = 0;
	Day = 0;
	Month = 0;
	Year = 0;
	DayofWeek = 0;
	Milliseconds = 0;
}

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
	return Second == 0 && Minute == 0 && Hour == 0 && Day == 0 && Month == 0 && Year == 0 && DayofWeek == 0 && Milliseconds == 0;
}

bool DateTime::parseHttpDate(String httpDate)
{
	char* ptr = (char*)httpDate.c_str();
	int first = httpDate.indexOf(',');
	if (first == -1 || httpDate.length() - first < 20) return false;
	first++; // Skip ','
	if (httpDate[first] == ' ') first ++;

	ptr += first;
	Day = (int8_t)strtol(ptr, &ptr, 10);
	if (*ptr == 0) return false;
	ptr++;
	char month[4] = {0};
	memcpy(month, ptr, 3);
	ptr += 4;
	if (*ptr == 0) return false;
	String mon = month;
	mon.toLowerCase();

	if (mon == "jan") Month = 0;
	else if (mon == "feb") Month = 1;
	else if (mon == "mar") Month = 2;
	else if (mon == "apr") Month = 3;
	else if (mon == "may") Month = 4;
	else if (mon == "jun") Month = 5;
	else if (mon == "jul") Month = 6;
	else if (mon == "aug") Month = 7;
	else if (mon == "sep") Month = 8;
	else if (mon == "oct") Month = 9;
	else if (mon == "nov") Month = 10;
	else if (mon == "dec") Month = 11;
	else return false;

	Year = (int16_t)strtol(ptr, &ptr, 10);
	if (*ptr == 0) return false;
	if (Year < 69)
		Year += 2000;
	else if (Year < 100)
		Year += 1900;

	Hour = (int8_t)strtol(ptr, &ptr, 10);
	if (*ptr != ':') return false;
	ptr++;
	Minute = (int8_t)strtol(ptr, &ptr, 10);
	if (*ptr != ':') return false;
	ptr++;
	Second = (int8_t)strtol(ptr, &ptr, 10);
	Milliseconds = 0;

	return true;
}

time_t DateTime::toUnixTime()
{
	return convertToUnixTime(Second + (Milliseconds / 1000), Minute, Hour, Day, Month, Year);
}

String DateTime::toShortDateString()
{
	char buf[64];
	sprintf(buf, "%02d.%02d.%d", Day, Month + 1, Year);
	return String(buf);
}

String DateTime::toShortTimeString(bool includeSeconds /* = false*/)
{
	char buf[64];
	if (includeSeconds)
		sprintf(buf, "%02d:%02d:%02d", Hour, Minute, Second);
	else
		sprintf(buf, "%02d:%02d", Hour, Minute);

	return String(buf);
}

String DateTime::toFullDateTimeString()
{
	return toShortDateString() + " " + toShortTimeString(true);
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

void DateTime::convertFromUnixTime(time_t timep, int8_t *psec, int8_t *pmin, int8_t *phour, int8_t *pday, int8_t *pwday, int8_t *pmonth, int16_t *pyear)
{
// convert the given time_t to time components
// this is a more compact version of the C library localtime function

  time_t long epoch=timep;
  int8_t year;
  int8_t month, monthLength;
  unsigned long days;
  
  *psec=epoch%60;
  epoch/=60; // now it is minutes
  *pmin=epoch%60;
  epoch/=60; // now it is hours
  *phour=epoch%24;
  epoch/=24; // now it is days
  *pwday=(epoch+4)%7;
  
  year=70;  
  days=0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch) {
    year++;
  }
  *pyear=year + 1900; // *pyear is returned as years from 1900
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  epoch -= days; // now it is days in this year, starting at 0
  //*pdayofyear=epoch;  // days since jan 1 this year
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (epoch>=monthLength) {
      epoch-=monthLength;
    } else {
        break;
    }
  }
  *pmonth=month;  // jan is month 0
  *pday=epoch+1;  // day of month
}


time_t DateTime::convertToUnixTime(int8_t sec, int8_t min, int8_t hour, int8_t day, int8_t month, int16_t year )
{
// converts time components to time_t 
// note year argument is full four digit year (or digits since 2000), i.e.1975, (year 8 is 2008)
  
   int i;
   time_t seconds;

   if(year < 69) 
      year+= 2000;
    // seconds from 1970 till 1 jan 00:00:00 this year
    seconds= (year-1970)*(60*60*24L*365);

    // add extra days for leap years
    for (i=1970; i<year; i++) {
        if (LEAP_YEAR(i)) {
            seconds+= 60*60*24L;
        }
    }
    // add days for this year
    for (i=0; i<month; i++) {
      if (i==1 && LEAP_YEAR(year)) { 
        seconds+= 60*60*24L*29;
      } else {
        seconds+= 60*60*24L*monthDays[i];
      }
    }

    seconds+= (day-1)*3600*24L;
    seconds+= hour*3600L;
    seconds+= min*60L;
    seconds+= sec;
    return seconds; 
}
