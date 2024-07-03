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
#include <WString.h>
#include "SmingLocale.h"
#include <sming_attr.h>

/* Useful Constants */
#define SECS_PER_MIN 60
#define SECS_PER_HOUR 3600
#define SECS_PER_DAY (SECS_PER_HOUR * 24)
#define MINS_PER_HOUR 60
#define DAYS_PER_WEEK 7
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52)
#define SECS_YR_2000 946681200

/** @brief  Days of week
*/
enum dtDays_t {
	dtSunday,
	dtMonday,
	dtTuesday,
	dtWednesday,
	dtThursday,
	dtFriday,
	dtSaturday,
};

/** @brief  Months
*/
enum dtMonth_t {
	dtJanuary,
	dtFebruary,
	dtMarch,
	dtApril,
	dtMay,
	dtJune,
	dtJuly,
	dtAugust,
	dtSeptember,
	dtOctober,
	dtNovember,
	dtDecember,
};

/* Useful functions for getting elapsed time */

/** Get just seconds part of given Unix time */
inline constexpr uint8_t numberOfSeconds(time_t time)
{
	return time % SECS_PER_MIN;
}

/** Get just minutes part of given Unix time */
inline constexpr uint8_t numberOfMinutes(time_t time)
{
	return (time / SECS_PER_MIN) % SECS_PER_MIN;
}

/** Get just hours part of given Unix time */
inline constexpr uint8_t numberOfHours(time_t time)
{
	return (time % SECS_PER_DAY) / SECS_PER_HOUR;
}

/** Get day of week from given Unix time */
inline constexpr dtDays_t dayOfWeek(time_t time)
{
	return dtDays_t((time / SECS_PER_DAY + 4) % DAYS_PER_WEEK);
}

/** Get elapsed days since 1970-01-01 from given Unix time */
inline constexpr uint8_t elapsedDays(time_t time)
{
	return time / SECS_PER_DAY;
}

/** Get quantity of seconds since midnight from given Unix time */
inline constexpr unsigned elapsedSecsToday(time_t time)
{
	return time % SECS_PER_DAY;
}

/** Get Unix time of midnight at start of day from given Unix time */
inline constexpr time_t previousMidnight(time_t time)
{
	return (time / SECS_PER_DAY) * SECS_PER_DAY;
}

/** Get Unix time of midnight at end of day from given just Unix time */
inline constexpr time_t nextMidnight(time_t time)
{
	return previousMidnight(time) + SECS_PER_DAY;
}

/** Get quantity of seconds since midnight at start of previous Sunday from given Unix time */
inline constexpr unsigned elapsedSecsThisWeek(time_t time)
{
	return elapsedSecsToday(time) + dayOfWeek(time) * SECS_PER_DAY;
}

/** @brief  Date and time class
 *
 * This class contains a 'broken-down' date and time into its individual year, month, etc. components.
 *
 * Date and time functions mostly work with Unix time, the quantity of seconds since 00:00:00 1970-01-01.
 * There is no support for leap seconds which are added (and in theory, removed) occasionally to compensate for earth rotation variation.
 * This means that timespan calculation and free-running clocks may be inaccurate if they span leap seconds.
 * To facilitate leap seconds, reference must be made to leap second table.
 * This will not be done within the Sming framework and must be handled by application code if required.
 * @see https://www.eecis.udel.edu/~mills/leap.html
 *
 * @note time_t is a signed 64-bit value for all architectures **except** Windows Host and esp32 ESP-IDF 4.x.
 *
 * 32-bit signed values support a range of +/-68 years; the Unix epoch is midnight 1 Jan 1970, so overflows at about 3am on 19 Jan 2038.
 * The value is signed which allows dates prior to 1970 to be represented.
 *
 * An unsigned 32-bit value can be used to store dates provided they are after 1970.
 * These are good until February 2106.
 */
class DateTime
{
public:
	/**
	 * @brief Basic information required when displaying or handling local times.
	 */
	struct ZoneInfo {
		/**
		 * @brief Type for timezone abbreviation such as "GMT", "EEST"
		 */
		struct Tag {
			static constexpr size_t maxSize = 5;
			char value[maxSize + 1];

			/**
			 * @name String will be truncated if required and always NUL terminated.
			 * @{
			 */
			static Tag fromString(const char* s);
			static Tag fromString(const char* s, size_t len);
			/** @} */

			operator const char*() const
			{
				return value;
			}
		};

		Tag tag{};			   ///< Abbreviation such as "GMT", "EEST" shown after time
		int16_t offsetMins{0}; ///< Offset from UTC in minutes
		bool isDst{false};	 ///< True if daylight savings is in effect

		/**
		 * @brief Get the offset in seconds so it can be added/subtracted directly from a time_t value
		 */
		int offsetSecs() const
		{
			return int(offsetMins) * SECS_PER_MIN;
		}

		/**
		 * @brief Return offset in ISO8601 string format, e.g. +11:00
		 */
		String getOffsetString(char sep) const;
	};

	/** @brief  Instantiate an uninitialised date and time object
	 */
	DateTime() = default;

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
		setTime(toUnixTime(sec, min, hour, day, month, year));
	}

	/** @brief  Parse a HTTP full date and set time and date
	 *  @param  httpDate HTTP full date in RFC 1123 format, e.g. Sun, 06 Nov 1994 08:49:37 GMT
	 *  @retval bool True on success
	 *  @see    See `fromHttpDate(const String& time_t&)`
	 */
	bool fromHttpDate(const String& httpDate);

	/** @brief  Parse a HTTP full date string and return the time_t value
	 *  @param  httpDate HTTP full date in RFC 1123 format, e.g. Sun, 06 Nov 1994 08:49:37 GMT
	 *  @param  time On success, contains the decoded value
	 *  @retval bool True on success
	 *  @note   Also supports obsolete RFC 850 date format, e.g. Sunday, 06-Nov-94 08:49:37 GMT where 2 digit year represents range 1970-2069
	 *  @note   GMT suffix is optional and is always assumed / ignored
	 */
	static bool fromHttpDate(const String& httpDate, time_t& time);

	/** @brief  Parse an ISO8601 date/time string
	 *  @param  datetime Date and optional time in ISO8601 format, e.g. "1994-11-06", "1994-11-06T08:49:37". Separators are optional.
	 *  @param  zone If provided, on success the `offsetMins` field will contain the time offset (0 for GMT) and the
	 * 			decoded DateTime will be 'local'. If zone is null then the decoded datetime will be UTC.
	 *  @retval bool True on success. On failure, value of DateTime is unchanged.
	 *  @see    See `fromISO8601(const String&, time_t&, uint16_t&, int16_t&)`
	 */
	bool fromISO8601(const String& datetime, ZoneInfo* zone = nullptr);

	/** @brief  Parse an ISO8601 date/time string and return discrete components
	 *  @param  datetime Date and optional time in ISO8601 format, e.g. "1994-11-06", "1994-11-06T08:49:37". Separators are optional.
	 *  @param  time The time_t component
	 *  @param  milliseconds Additional milliseconds value
	 *  @param  offsetMins Any offset specified in the time
	 *  @retval bool True on success
	 *  @see    See https://en.wikipedia.org/wiki/ISO_8601
	 *
	 * `Basic format` doesn't include separators, whereas `Extended format` does. Both are supported.
	 *
	 * Acceptable date formats:
	 *
	 * 	YYYY-MM-DD or YYYYMMDD
	 * 	YYYY-MM (but not YYYYMM)
	 *
	 * Acceptable time formats:
	 *
	 * 	Thh:mm:ss.sss or Thhmmss.sss
	 * 	Thh:mm:ss or Thhmmss
	 * 	Thh:mm.mmm or Thhmm.mmm
	 * 	Thh:mm or Thhmm
	 * 	Thh.hhh
	 * 	Thh
	 *
	 * Times with an offset:
	 *
	 *  <time>Z
	 * 	<time>±hh:mm
	 * 	<time>±hhmm
	 * 	<time>±hh
	 */
	static bool fromISO8601(const String& datetime, time_t& time, uint16_t& milliseconds, int16_t& offsetMins);

	/** @brief  Check if time date object is initialised
	 *  @retval True if object has no value. False if initialised.
	 */
	bool isNull() const;

	/** @brief  Get Unix time
	 *  @retval time_t Unix time, quantity of seconds since 00:00:00 1970-01-01
	 *  @note   Unix time does not account for leap seconds.
	 */
	time_t toUnixTime() const;

	/** @brief  Get human readable date
	 *  @retval String Date in requested format, e.g. DD.MM.YYYY
	 */
	String toShortDateString() const;

	/** @brief  Get human readable time
	 *  @param  includeSeconds True to include seconds (Default: false)
	 *  @retval String Time in format hh:mm or hh:mm:ss
	 */
	String toShortTimeString(bool includeSeconds = false) const;

	/** @brief  Get human readable date and time
	 *  @retval String Date and time in format DD.MM.YYYY hh:mm:ss
	 */
	String toFullDateTimeString() const;

	/** @brief  Get human readable date and time
	 *  @param  zone Optional timezone information
	 *  @retval String Date and time
	 *  @see    See `fromISO8601()` for string formats
	 *  @note   If `zone` isn't specified then UTC is assumed and timezone indicator 'Z' will be appended
	 */
	String toISO8601(const ZoneInfo* zone = nullptr) const;

	/** @brief  Get human readable date and time
	 *  @retval String Date and time in format DDD, DD MMM YYYY hh:mm:ss GMT
	 */
	String toHTTPDate() const;

	/** @brief  Add time to date time object
	 *  @param  add Quantity of milliseconds to add to object
	 *  @note   This operation is computationally expensive, requiring conversion to and from time_t.
	 */
	void addMilliseconds(long add);

	/** @brief  Convert from Unix time to individual time components
	 *  @param  timep Unix time date value to convert
	 *  @param  psec Pointer to integer to hold resulting seconds
	 *  @param  pmin Pointer to integer to hold resulting minutes
	 *  @param  phour Pointer to integer to hold resulting hour
	 *  @param  pday Pointer to integer to hold resulting day of month
	 *  @param  pwday Pointer to integer to hold resulting day of week
	 *  @param  pmonth Pointer to integer to hold resulting month
	 *  @param  pyear Pointer to integer to hold resulting year
	 *  @note   Pass the Unix timedate value and pointers to existing integers. The integers are updated with the converted values
	 *  @note   This static function  may be used without instantiating a DateTime object, e.g. DateTime::convertFromUnixTime(...);
	 *  @note   Unix time does not account for leap seconds.
	 *  @note   All of the return values are optional, specify nullptr if not required
	 */
	static void fromUnixTime(time_t timep, uint8_t* psec, uint8_t* pmin, uint8_t* phour, uint8_t* pday, uint8_t* pwday,
							 uint8_t* pmonth, uint16_t* pyear);

	/** @brief  Convert from individual time components to Unix time
	 *  @param  sec Seconds
	 *  @param  min Minutes
	 *  @param  hour Hours
	 *  @param  day Days
	 *  @param  month Month (0-11, Jan=0, Feb=1, ...Dec=11), or enum (dtJanuary, ...)
	 *  @param  year Year, either full 4 digit year or 2 digits for 2000-2068
	 *  @retval time_t Number of seconds since unix epoch (Midnight, Jan 1 1970)
	 *  @note   Seconds, minutes, hours and days may be any value, e.g. to calculate the value for 300 days since 1970 (epoch), set day=300
	 *  @note   This static function  may be used without instantiating a DateTime object, e.g. `time_t unixTime = DateTime::toUnixTime(...);`
	 *  @note   Unix time does not account for leap seconds.
	 */
	static time_t toUnixTime(int sec, int min, int hour, int day, uint8_t month, uint16_t year);

	/** @brief  Create string formatted with time and date placeholders
	 *  @param  formatString String including date and time formatting
	 *  @param  zone Optional timezone information
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
	 *  | %%z   | Timezone offset in ±HHMM format (1) |  |
	 *  | %%:z  | Timezone offset in ±HH:MM format (1) |  |
	 *  | %%Z   | Timezone tag (1) |  |
	 *  | %%    | Percent sign |  |
	 *
	 * (1) If zone is not provided then the %z, %:z and %Z placeholders produce empty text
	 */
	String format(const char* formatString, const ZoneInfo* zone = nullptr) const;

	/** @brief  Create string formatted with time and date placeholders
	 *  @param  formatString String including date and time formatting
	 *  @param  zone Optional zone information containing additional format information
	 *  @retval String Formatted string
	 *  @see 	See `format(const char*, const ZoneInfo*)` for parameter details
	 */
	String format(const String& formatString, const ZoneInfo* zone = nullptr) const
	{
		return format(formatString.c_str(), zone);
	}

	/**
	 * @name Utility functions
	 * @{
	 */
	static bool isLeapYear(uint16_t year);
	static uint8_t getMonthDays(uint8_t month, uint16_t year);
	static String getLocaleDayName(uint8_t day);
	static String getLocaleMonthName(uint8_t month);
	static String getIsoDayName(uint8_t day);
	static String getIsoMonthName(uint8_t month);
	static uint16_t getDaysInYear(uint16_t year);
	/** @} */

private:
	// Helper methods
	void calcDayOfYear();

	// Calculate week number based on firstDay of week
	uint8_t getWeekOfYear(dtDays_t firstDay) const;

public:
	uint16_t Year = 0;		   ///< Full Year number
	uint16_t DayofYear = 0;	///< Day of year (0-365)
	uint8_t DayofWeek = 0;	 ///< Day of week (0-6 Sunday is day 0)
	uint8_t Month = 0;		   ///< Month (0-11 Jan is month 0)
	uint8_t Day = 0;		   ///< Day of month (1-31)
	uint8_t Hour = 0;		   ///< Hour (0-23)
	uint8_t Minute = 0;		   ///< Minute (0-59)
	uint8_t Second = 0;		   ///< Second (0-59)
	uint16_t Milliseconds = 0; ///< Milliseconds (0-999)
};

/** @} */
