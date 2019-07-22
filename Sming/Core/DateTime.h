/*
  DateTime.h - Arduino library for date and time functions
  Copyright (c) Michael Margolis.  All right reserved.


  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ///
  Heavily rewritten for Sming Framework
*/

/** @defgroup datetime Date and time functions
 *  @{
 */
#pragma once

#include <time.h>
#include "WString.h"
#include "SmingLocale.h"

/*==============================================================================*/
/* Useful Constants */
#define SECS_PER_MIN (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY (SECS_PER_HOUR * 24L)
#define DAYS_PER_WEEK (7L)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52L)
#define SECS_YR_2000 (946681200UL)

/* Useful Macros for getting elapsed time */
/** Get just seconds part of given Unix time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
/** Get just minutes part of given Unix time */
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
/** Get just hours part of given Unix time */
#define numberOfHours(_time_) ((_time_ % SECS_PER_DAY) / SECS_PER_HOUR)
/** Get day of week from given Unix time */
#define dayOfWeek(_time_) ((_time_ / SECS_PER_DAY + 4) % DAYS_PER_WEEK) // 0 = Sunday
/** Get elapsed days since 1970-01-01 from given Unix time */
#define elapsedDays(_time_) (_time_ / SECS_PER_DAY) // this is number of days since Jan 1 1970
/** Get quantity of seconds since midnight from given Unix time */
#define elapsedSecsToday(_time_) (_time_ % SECS_PER_DAY) // the number of seconds since last midnight
/** Get Unix time of midnight at start of day from given Unix time */
#define previousMidnight(_time_) ((_time_ / SECS_PER_DAY) * SECS_PER_DAY) // time at the start of the given day
/** Get Unix time of midnight at end of day from given just Unix time */
#define nextMidnight(_time_) (previousMidnight(_time_) + SECS_PER_DAY) // time at the end of the given day
/** Get quantity of seconds since midnight at start of previous Sunday from given Unix time */
#define elapsedSecsThisWeek(_time_) (elapsedSecsToday(_time_) + (dayOfWeek(_time_) * SECS_PER_DAY))

// todo add date math macros
/*============================================================================*/

/** @brief  Days of week
*/
typedef enum {
	dtSunday,	///< Sunday
	dtMonday,	///< Monday
	dtTuesday,   ///< Tuesday
	dtWednesday, ///< Wednesday
	dtThursday,  ///< Thursday
	dtFriday,	///< Friday
	dtSaturday   ///< Saturday
} dtDays_t;

/** @brief  Date and time class
 *
 *          Date and time functions mostly work with Unix time, the quantity of seconds since 00:00:00 1970-01-01.
 *          There is no support for leap seconds which are added (and in theory, removed) occasionally to compensate for earth rotation variation.
 *          This means that timespan calculation and free-running clocks may be inaccurate if they span leap seconds.
 *          To facilitate leap seconds, reference must be made to leap second table. This will not be done within the Sming framework and must be handled by application code if required.
 *  @note   Sming uses 32-bit signed integer for its time_t data type which supports a range of +/-68 years. This means Sming is susceptible to Year 2038 problem.
 */
class DateTime
{
public:
	/** @brief  Instantiate an uninitialised date and time object
	 */
	DateTime()
	{
	}

	/** @brief  Instantiate a date and time object from a Unix timestamp
	 *  @param  time Unix time to assign to object
	 */
	DateTime(time_t time)
	{
		setTime(time);
	}

	/** @brief  Get current Unix time
	 *  @retval time_t Quantity of seconds since 00:00:00 1970-01-01
	 */
	operator time_t()
	{
		return toUnixTime();
	}

	/** @brief  Set time using Unix timestamp
     *  @param  time Unix time to set object time to
     */
	void setTime(time_t time);

	/** @brief  Set time using time and date component values
	 *  @param  sec Seconds
	 *  @param  min Minutes
	 *  @param  hour Hour
	 *  @param  day Day of month
	 *  @param  month Month (0=Jan, 11=Dec)
	 *  @param  year Year
	 */
	void setTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year)
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

	/** @brief  Parse a HTTP full date and set time and date
	 *  @param  httpDate HTTP full date in RFC 1123 format, e.g. Sun, 06 Nov 1994 08:49:37 GMT
	 *  @retval bool True on success
	 *  @note   Also supports obsolete RFC 850 date format, e.g. Sunday, 06-Nov-94 08:49:37 GMT where 2 digit year represents range 1970-2069
	 *  @note   GMT suffix is optional and is always assumed / ignored
	 */
	bool fromHttpDate(const String& httpDate);

	/** @brief  Parse a HTTP full date and set time and date
	 *  @param  httpDate HTTP full date in RFC 1123 format, e.g. Sun, 06 Nov 1994 08:49:37 GMT
	 *  @retval bool True on success
	 *  @note   Also supports obsolete RFC 850 date format, e.g. Sunday, 06-Nov-94 08:49:37 GMT where 2 digit year represents range 1970-2069
	 *  @note   GMT suffix is optional and is always assumed / ignored
	 *  @deprecated Use `fromHttpDate()` instead
	 */
	bool parseHttpDate(const String& httpDate) SMING_DEPRECATED
	{
		return fromHttpDate(httpDate);
	}

	/** @brief  Check if time date object is initialised
	 *  @retval True if object has no value. False if initialised.
	 */
	bool isNull();

	/** @brief  Get Unix time
	 *  @retval time_t Unix time, quantity of seconds since 00:00:00 1970-01-01
	 *  @note   Unix time does not account for leap seconds. To convert Unix time to UTC requires reference to a leap second table.
	 */
	time_t toUnixTime();

	/** @brief  Get human readable date
	 *  @retval String Date in requested format, e.g. DD.MM.YYYY
	 */
	String toShortDateString();

	/** @brief  Get human readable time
	 *  @param  includeSeconds True to include seconds (Default: false)
	 *  @retval String Time in format hh:mm or hh:mm:ss
	 */
	String toShortTimeString(bool includeSeconds = false);

	/** @brief  Get human readable date and time
	 *  @retval String Date and time in format DD.MM.YYYY hh:mm:ss
	 */
	String toFullDateTimeString();

	/** @brief  Get human readable date and time
	 *  @retval String Date and time in format YYYY-MM-DDThh:mm:ssZ
	 */
	String toISO8601();

	/** @brief  Get human readable date and time
	 *  @retval String Date and time in format DDD, DD MMM YYYY hh:mm:ss GMT
	 */
	String toHTTPDate();

	/** @brief  Add time to date time object
	 *  @param  add Quantity of milliseconds to add to object
	 */
	void addMilliseconds(long add);

	// functions to convert to and from time components (hrs, secs, days, years etc) to time_t
	/** @brief  Convert from Unix time to individual time components
	 *  @param  timep Unix time date value to convert
	 *  @param  psec Pointer to integer to hold resulting seconds
	 *  @param  pmin Pointer to integer to hold resulting minutes
	 *  @param  phour Pointer to integer to hold resulting hour
	 *  @param  pday Pointer to integer to hold resulting day of month
	 *  @param  pwday Pointer to integer to hold resulting day of week
	 *  @param  pmonth Pointer to integer to hold resulting month
	 *  @param  pyear Pointer to integer to hold resulting year
	 *  @note   This is a more compact version of the C library localtime function
	 *  @note   Pass the Unix timedate value and pointers to existing integers. The integers are updated with the converted values
	 *  @note   This static function  may be used without instantiating a DateTime object, e.g. DateTime::convertFromUnixTime(...);
	 *  @note   32-bit Unix time has year 2036 issue.
	 *  @note   Unix time does not account for leap seconds. To convert Unix time to UTC requires reference to a leap second table.
	 *  @note   All of the return values are optional, specify nullptr if not required
	 */
	static void fromUnixTime(time_t timep, uint8_t* psec, uint8_t* pmin, uint8_t* phour, uint8_t* pday, uint8_t* pwday,
							 uint8_t* pmonth, uint16_t* pyear);

	/** @deprecated Use unsigned version instead
	 * 		`fromUnixTime(time_t, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint16_t*)` */
	static void fromUnixTime(time_t timep, int8_t* psec, int8_t* pmin, int8_t* phour, int8_t* pday, int8_t* pwday,
							 int8_t* pmonth, int16_t* pyear) SMING_DEPRECATED
	{
		fromUnixTime(timep, reinterpret_cast<uint8_t*>(psec), reinterpret_cast<uint8_t*>(pmin),
					 reinterpret_cast<uint8_t*>(phour), reinterpret_cast<uint8_t*>(pday),
					 reinterpret_cast<uint8_t*>(pwday), reinterpret_cast<uint8_t*>(pmonth),
					 reinterpret_cast<uint16_t*>(pyear));
	}

	// functions to convert to and from time components (hrs, secs, days, years etc) to time_t
	/** @brief  Convert from Unix time to individual time components
	 *  @param  timep Unix time date value to convert
	 *  @param  psec Pointer to integer to hold resulting seconds
	 *  @param  pmin Pointer to integer to hold resulting minutes
	 *  @param  phour Pointer to integer to hold resulting hour
	 *  @param  pday Pointer to integer to hold resulting day of month
	 *  @param  pwday Pointer to integer to hold resulting day of week
	 *  @param  pmonth Pointer to integer to hold resulting month
	 *  @param  pyear Pointer to integer to hold resulting year
	 *  @note   This is a more compact version of the C library localtime function
	 *  @note   Pass the Unix timedate value and pointers to existing integers. The integers are updated with the converted values
	 *  @note   This static function  may be used without instantiating a DateTime object, e.g. DateTime::convertFromUnixTime(...);
	 *  @note   32-bit Unix time has year 2036 issue.
	 *  @note   Unix time does not account for leap seconds. To convert Unix time to UTC requires reference to a leap second table.
	 *  @note   All of the return values are optional, specify nullptr if not required
	 *  @deprecated Use `fromUnixTime(time_t, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint16_t*)` instead
	 */
	static void convertFromUnixTime(time_t timep, int8_t* psec, int8_t* pmin, int8_t* phour, int8_t* pday,
									int8_t* pwday, int8_t* pmonth, int16_t* pyear) SMING_DEPRECATED
	{
		fromUnixTime(timep, reinterpret_cast<uint8_t*>(psec), reinterpret_cast<uint8_t*>(pmin),
					 reinterpret_cast<uint8_t*>(phour), reinterpret_cast<uint8_t*>(pday),
					 reinterpret_cast<uint8_t*>(pwday), reinterpret_cast<uint8_t*>(pmonth),
					 reinterpret_cast<uint16_t*>(pyear));
	}

	/** @brief  Convert from individual time components to Unix time
	 *  @param  sec Seconds
	 *  @param  min Minutes
	 *  @param  hour Hours
	 *  @param  day Days
	 *  @param  month Month (0-11, Jan=0, Feb=1, ...Dec=11)
	 *  @param  year Year (1901-2036), either full 4 digit year or 2 digits for 1970-2036
	 *  @note   Seconds, minutes, hours and days may be any value, e.g. to calculate the value for 300 days since 1970 (epoch), set day=300
	 *  @note   This static function  may be used without instantiating a DateTime object, e.g. time_t unixTime = DateTime::convertToUnixTime(...);
	 *  @note   32-bit Unix time is valid between 1901-12-13 and 03:14:07 2038-01-19
	 *  @note   Unix time does not account for leap seconds. To convert Unix time to UTC requires reference to a leap second table.
	 */
	static time_t toUnixTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year);

	/** @brief  Convert from individual time components to Unix time
	 *  @param  sec Seconds
	 *  @param  min Minutes
	 *  @param  hour Hours
	 *  @param  day Days
	 *  @param  month Month (0-11, Jan=0, Feb=1, ...Dec=11)
	 *  @param  year Year (1901-2036), either full 4 digit year or 2 digits for 1970-2036
	 *  @note   Seconds, minutes, hours and days may be any value, e.g. to calculate the value for 300 days since 1970 (epoch), set day=300
	 *  @note   This static function  may be used without instantiating a DateTime object, e.g. time_t unixTime = DateTime::convertToUnixTime(...);
	 *  @note   32-bit Unix time is valid between 1901-12-13 and 03:14:07 2038-01-19
	 *  @note   Unix time does not account for leap seconds. To convert Unix time to UTC requires reference to a leap second table.
	 *  @deprecated Use 'toUnixTime()' instead
	 */
	static time_t convertToUnixTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month,
									uint16_t year) SMING_DEPRECATED
	{
		return toUnixTime(sec, min, hour, day, month, year);
	}

	/** @brief  Create string formatted with time and date placeholders
	 *  @param  formatString String including date and time formatting
	 *  @retval String Formatted string
	 *  @note   Uses strftime style formatting, e.g. format("Today is %a, %d %b %Y") returns "Today is Mon, 10 Dec 2018"
	 *  @note   Localisation may be implemented in libsming at compile time by setting LOCALE, e.g. LOCALE=LOCALE_DE_DE
	 *	@note   Default localisation is EN_GB
	 *  @note   Formatting parameters
	 *  | Param | Description | Locale |
	 *  | :----:| :---------- | :----: |
	 *  | %%a   | Abbreviated weekday name| * |
	 *  | %%A   | Full weekday name | * |
	 *  | %%b   | Abbreviated month name | * |
	 *  | %%B   | Full month name | * |
	 *  | %%c   | Locale preferred date and time format | * |
	 *  | %%C   | Century number (2 digits) |  |
	 *  | %%d   | Day of month as decimal number with leading zero (2 digits) |  |
	 *  | %%D   | US date format (mm/dd/yyyy) |  |
	 *  | %%e   | Day of month as decimal number with leading space (2 digits) |  |
	 *  | %%F   | ISO 8601 date format (YYYY-mm-dd) |  |
	 *  | %%h   | Equivalent to %%b | * |
	 *  | %%H   | Hour as a decimal number using a 24-hour clock (range 00 to 23) |  |
	 *  | %%I   | Hour as a decimal number using a 12-hour clock (range 00 to 12) |  |
	 *  | %%j   | Day of the year as a decimal number (range 001 to 366) |  |
	 *  | %%m   | Month as a decimal number (range 01 to 12) |  |
	 *  | %%M   | Minute as a decimal number (range 00 to 59) |  |
	 *  | %%n   | Newline |  |
	 *  | %%p   | Meridiem indicator: AM or PM. Midnight is AM and noon is PM |  |
	 *  | %%r   | Locale 12-hour clock time notation. This is equivalent to %%I:%%M:%%S %%p | * |
	 *  | %%R   | Time in 24-hour notation (HH:MM) |  |
	 *  | %%S   | Second as a decimal number (range 00 to 60) |  |
	 *  | %%t   | Horizontal tab |  |
	 *  | %%T   | Time in 24-hour notation (HH:MM:SS) |  |
	 *  | %%u   | Day of the week as a decimal (range 1 to 7, Monday is 1) |  |
	 *  | %%U   | Week number as a decimal number (range 00 to 53, first Sunday as the first day of week 01) |  |
	 *  | %%V   | ISO 8601 week number as a decimal number (range 01 to 53, where week 1 is the first week including a Thursday) |  |
	 *  | %%w   | Day of the week as a decimal (range 0 to 6, Sunday is 0) |  |
	 *  | %%W   | Week number as a decimal number (range 00 to 53, first Monday as the first day of week 01) |  |
	 *  | %%x   | Locale preferred date representation | * |
	 *  | %%X   | Locale preferred time representation | * |
	 *  | %%y   | Year as a decimal number without a century (range 00 to 99) |  |
	 *  | %%Y   | Year as a decimal number (range 1970 to ...) |  |
	 *  | %%    | Percent sign |  |
	 */
	String format(const char* formatString);

	/** @brief  Create string formatted with time and date placeholders
	 *  @param  formatString String including date and time formatting
	 *  @retval String Formatted string
	 *  @note see format(const char*) for parameter details
	 */
	String format(const String& formatString)
	{
		return format(formatString.c_str());
	}

private:
	void calcDayOfYear();				// Helper function calculates day of year
	uint8_t calcWeek(uint8_t firstDay); //Helper function calculates week number based on firstDay of week

public:
	uint8_t Hour = 0;		   ///< Hour (0-23)
	uint8_t Minute = 0;		   ///< Minute (0-59)
	uint8_t Second = 0;		   ///< Second (0-59)
	uint16_t Milliseconds = 0; ///< Milliseconds (0-999)
	uint8_t Day = 0;		   ///< Day of month (1-31)
	uint8_t DayofWeek = 0;	 ///< Day of week (0-6 Sunday is day 0)
	uint16_t DayofYear = 0;	///< Day of year (0-365)
	uint8_t Month = 0;		   ///< Month (0-11 Jan is month 0)
	uint16_t Year = 0;		   ///< Full Year number
};

/** @} */
