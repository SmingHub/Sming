#pragma once

#include "DateTime.h"
#include <Data/CString.h>

/**
 * @brief A timestamp representing a UTC 'point in time' associated with a specific timezone
 *
 * Derivation of the correct `DateTime::ZoneInfo` value is complex and typically managed by
 * an external library, such as :library:`Timezone`.
 *
 * The implicit `time_t` value for a ZonedTime is the corresponding UTC time.
 * All comparisons or arithmetic operations will therefore operate using this value.
 * This means two ZonedTime objects will be considered equal if their corresponding UTC
 * timestamps match, even if they derive from different timezones.
 *
 * To obtain the `time_t` value with local offset applied, use `local()`:
 *
 * 		DateTime dtLocal(zonedTime.local());
 *
 * There is no support in this class for manipulating time or zone information directly.
 * This should be done using the raw `time_t` value and a new ZonedTime object constructed.
 */
class ZonedTime
{
public:
	/**
	 * @brief Construct a zoned time
	 * @param utc
	 * @param zoneInfo
	 */
	ZonedTime(time_t utc, const DateTime::ZoneInfo& zoneInfo) : utc(utc), zoneInfo(zoneInfo)
	{
	}

	/**
	 * @brief Construct a time in the UTC zone
	 */
	explicit ZonedTime(time_t utc = 0) : utc(utc), zoneInfo{{"UTC"}}
	{
	}

	ZonedTime(const ZonedTime& other) = default;

	operator time_t() const
	{
		return utc;
	}

	/**
	 * @brief Get a ZonedTime instance translated into the UTC zone
	 * This does nothing more than replace the contained zone information with UTC,
	 * which has tag "UTC", no offset and no daylight savings.
	 */
	ZonedTime toUtc() const
	{
		return ZonedTime(utc);
	}

	/**
	 * @brief Obtain the `time_t` value adjusted for local time
	 */
	time_t local() const
	{
		return utc + zoneInfo.offsetSecs();
	}

	/**
	 * @brief Get instance zone information
	 */
	const DateTime::ZoneInfo& getZoneInfo() const
	{
		return zoneInfo;
	}

	const DateTime::ZoneInfo::Tag& tag() const
	{
		return zoneInfo.tag;
	}

	int16_t offsetMins() const
	{
		return zoneInfo.offsetMins;
	}

	int offsetSecs() const
	{
		return zoneInfo.offsetSecs();
	}

	bool isDst() const
	{
		return zoneInfo.isDst;
	}

	/**
	 * @name Format time using `DateTime`
	 * @see   See `DateTime::format()` for details
	 * @{
	 */
	String format(const char* formatString) const
	{
		DateTime dt(local());
		return dt.format(formatString, &zoneInfo);
	}

	String format(const String& formatString) const
	{
		return format(formatString.c_str());
	}

	String toISO8601() const
	{
		DateTime dt(local());
		return dt.toISO8601(&zoneInfo);
	}
	/** @} */

	/**
	 * @brief Obtain default formatted string
	 */
	String toString() const
	{
		return format(defaultFormat ? defaultFormat.c_str() : _F("%a %d %b %Y %T %Z"));
	}

	/**
	 * @brief Set the default format for displaying zoned times
	 */
	static void setDefaultFormat(const String& format)
	{
		defaultFormat = format;
	}

private:
	static CString defaultFormat;
	time_t utc;
	DateTime::ZoneInfo zoneInfo;
};
