/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * NanoTime.h - Utilities for handling time periods at nanosecond resolution
 *
 * @author mikee47 <mike@sillyhouse.net>
 *
 * Note: C++ provides the `chrono` utilities but is not well-suited for embedded applications:
 *
 * - Only uses 64-bit calculations. NanoTime can use any integral type, although uint32_t is the most useful.
 * - Time conversions are truncated. A value of 0.99 seconds would be treated as 0.
 *   NanoTime rounds results so would return 1.
 * - Supports compile-time calculations but no runtime calculation support.
 *
 ****/

#pragma once

#include <cstdint>
#include <esp_attr.h>
#include <sming_attr.h>
#include <cmath>
#include "Rational.h"
#include <WString.h>

/** @defgroup   timers Timers
 *  @brief      System timer support classes
*/

/** @defgroup   system_clocks System clocks
 *  @brief System clock definitions
 *  @ingroup    timers
 *  @{
*/

namespace NanoTime
{
/**
 * @brief Identify units for a scalar quantity of time
 * @note Ordered in increasing unit size, e.g. days > seconds
 */
enum Unit {
	Nanoseconds,
	Microseconds,
	Milliseconds,
	Seconds,
	Minutes,
	Hours,
	Days,
	UnitMax = Days,
};

/**
 * @brief Get a string identifying the given time units, e.g. "ns"
 */
const char* unitToString(Unit unit);

/**
 * @brief Get a long string identifying the given time units, e.g. "seconds"
 */
const char* unitToLongString(Unit unit);

/**
 * @brief Class to represent a frequency
 */
struct Frequency {
	Frequency(uint32_t frequency) : frequency(frequency)
	{
	}

	operator uint32_t()
	{
		return frequency;
	}

	/**
	 * @brief Get frequency as compact string
	 * @note Drop trailing zeroes to produce a more compact string, e.g.
	 *		0 -> 0Hz
	 *		1000 -> 1KHz
	 *		4553000 -> 4553KHz
	 *		1000000000 -> 1GHz
	 *
	 */
	String toString() const;

	uint32_t frequency;
};

/**
 * @brief List of clock ticks for each supported unit of time
 */
constexpr BasicRatio32 unitTicks[UnitMax + 1] = {
	{1000000000, 1},   // Nanoseconds
	{1000000, 1},	  // Microseconds
	{1000, 1},		   // Milliseconds
	{1, 1},			   // Seconds
	{1, 60},		   // Minutes
	{1, 60 * 60},	  // Hours
	{1, 24 * 60 * 60}, // Days
};

/**
 * @brief Class template to define tick std::ratio type
 * @tparam unit
 * @retval std::ratio Ticks per second
 * @note This would be preferable:
 * `template <Unit unit> using UnitTickRatio = std::ratio<unitTicks[unit].num, unitTicks[unit].den>;`
 * But GCC 4.8 doesn't like it (lvalue required as unary '&' operand)
 */
template <Unit unit> struct UnitTickRatio {
	static constexpr uint64_t num = unitTicks[unit].num;
	static constexpr uint64_t den = unitTicks[unit].den;
};

// Forward declarations
template <class Clock_, Unit unit_, uint64_t time_> struct TimeConst;
template <class Clock_, uint64_t ticks_> struct TicksConst;
template <class Clock_, Unit unit_, typename TimeType_> struct TimeSource;
template <typename T> struct Time;
template <typename Clock_, typename T> struct Ticks;

/**
 * @brief Class template representing a physical Clock with fixed timing characteristics
 * @tparam ClockDef The actual Clock being constructed (so we can query its properties)
 * @tparam frequency_ Clock frequency in Hz
 * @tparam TickType_ Variable type representing the clock's tick value
 * @tparam maxTicks_ Maximum count value for the clock
 * @note Physical clocks are implemented using this as a base.
 * The provided frequency accounts for any prescaler setting in force. Fixing this
 * at compile time helps to avoid expensive runtime calculations and permits static range checks.
 * @see Use `TimeSource` to work with a Clock in specific time units
 */
template <typename ClockDef, uint32_t frequency_, typename TickType_, TickType_ maxTicks_> struct Clock {
	using TickType = TickType_;
	template <uint64_t ticks> using TicksConst = TicksConst<Clock, ticks>;
	template <Unit unit, uint64_t time> using TimeConst = TimeConst<Clock, unit, time>;
	template <Unit unit> using TicksPerUnit = std::ratio_divide<std::ratio<frequency_>, UnitTickRatio<unit>>;
	template <Unit unit, typename TimeType> using TimeSource = TimeSource<Clock, unit, TimeType>;
	template <typename T> using Ticks = Ticks<Clock, T>;

	static constexpr const char* typeName()
	{
		return ClockDef::typeName();
	}

	static constexpr uint32_t frequency()
	{
		return frequency_;
	}

	using MaxTicks = TicksConst<maxTicks_>;
	static constexpr MaxTicks maxTicks()
	{
		return MaxTicks();
	}

	template <Unit unit> using MaxTime = typename MaxTicks::template TimeConst<unit>;
	template <Unit unit> static constexpr MaxTime<unit> maxTime()
	{
		return MaxTime<unit>();
	}

	/**
	 * @brief Get ticks per unit as a Ratio object
	 * @retval BasicRatio32
	 */
	static Ratio32 ticksPerUnit(Unit unit)
	{
		const auto& tpu = unitTicks[unit];
		return Ratio32(frequency_ * tpu.den, tpu.num);
	}

	/**
	 * @brief Class template defining a fixed time quantity
	 * @tparam time
	 * @retval TimeConst
	 */
	template <Unit unit, uint64_t time> static constexpr TimeConst<unit, time> timeConst()
	{
		return TimeConst<unit, time>();
	}

	/**
	 * @brief Class template defining a fixed tick quantity
	 * @tparam ticks
	 * @retval TicksConst<Clock, ticks>
	 */
	template <uint64_t ticks> static constexpr TicksConst<ticks> ticksConst()
	{
		return TicksConst<ticks>();
	}

	/**
	 * @brief Create a Time Source for this Clock
	 * @tparam unit
	 * @tparam TimeType
	 */
	template <Unit unit, typename TimeType> static constexpr TimeSource<unit, TimeType> timeSource()
	{
		return TimeSource<unit, TimeType>();
	}

	/**
	 * @brief Get the number of ticks for a given time
	 * @param time
	 * @retval TimeType Tick count, rounded to the nearest tick
	 */
	template <Unit unit, typename TimeType> static Ticks<TimeType> timeToTicks(TimeType time)
	{
		return TimeSource<unit, TimeType>::timeToTicks(time);
	}

	/**
	 * @brief Get the time for a given number of clock ticks
	 * @param ticks
	 * @retval TimeType Time count, rounded to the nearest unit
	 */
	template <Unit unit, typename TimeType> static Time<TimeType> ticksToTime(TimeType ticks)
	{
		return TimeSource<unit, TimeType>::ticksToTime(ticks);
	}

	static String toString()
	{
		String s;
		s += typeName();
		s += '/';
		s += Frequency(frequency()).toString();
		return s;
	}
};

/**
 * @brief Function template to convert a constant time quantity from one unit to another
 * @tparam time The time to convert
 * @tparam unitsFrom Units for `time` parameter
 * @tparam unitsTo Units for return value
 * @retval TimeType Converted time
 * @note example:
 *
 * 	uint32_t micros = convert<50, Milliseconds, Microseconds>();
 */
template <uint64_t time, Unit unitsFrom, Unit unitsTo,
		  typename R = std::ratio_divide<UnitTickRatio<unitsTo>, UnitTickRatio<unitsFrom>>>
constexpr uint64_t convert()
{
	return ({ round(double(time) * R::num / R::den); });
}

/**
 * @brief Function template to convert a time quantity from one unit to another
 * @tparam TimeType Variable type to use for calculation
 * @param time The time to convert
 * @param unitsFrom Units for `time` parameter
 * @param unitsTo Units for return value
 * @retval TimeType Converted time, returns TimeType(-1) if calculation overflowed
 */
template <typename TimeType> __forceinline TimeType convert(const TimeType& time, Unit unitsFrom, Unit unitsTo)
{
	if(unitsFrom == unitsTo) {
		return time;
	}

	using R = Ratio<TimeType>;
	auto ratio = R(unitTicks[unitsTo]) / R(unitTicks[unitsFrom]);
	return time * ratio;
}

/**
 * @brief A time time broken into its constituent elements
 * @note Useful for analysing and printing time values
 */
struct TimeValue {
	TimeValue() = default;

	/**
	 * @brief Resolve a time value into constituent components
	 * @param time The time to resolve
	 * @param unit Units for given time
	 */
	template <typename TimeType> TimeValue(Unit unit, TimeType time)
	{
		set(unit, time);
	}

	template <typename TimeType> void set(Unit unit, TimeType time);

	/**
	 * @brief Get sub-second time entirely in microseconds
	 */
	uint32_t getMicroseconds() const
	{
		return microseconds + (milliseconds * 1000);
	}

	/**
	 * @brief Get sub-second time entirely in nanoseconds
	 */
	uint32_t getNanoseconds() const
	{
		return nanoseconds + getMicroseconds() * 1000;
	}

	String toString() const;

	operator String() const
	{
		return toString();
	}

	bool overflow = false;
	Unit unit = Seconds; ///< Time unit passed to set() call
	uint32_t days = 0;
	uint8_t hours = 0;
	uint8_t minutes = 0;
	uint8_t seconds = 0;
	uint16_t milliseconds = 0;
	uint32_t microseconds = 0;
	uint32_t nanoseconds = 0;
};

template <typename TimeType> void TimeValue::set(Unit unit, TimeType time)
{
	overflow = (time == TimeType(-1));
	this->unit = unit;

	TimeType elem[UnitMax + 1] = {0};
	elem[unit] = time;

	auto divmod = [&elem](Unit u, uint16_t div) {
		elem[u + 1] = elem[u] / div;
		elem[u] %= div;
	};

	if(unit < Days) {
		if(unit < Hours) {
			if(unit < Minutes) {
				if(unit < Seconds) {
					if(unit < Milliseconds) {
						if(unit < Microseconds) {
							divmod(Nanoseconds, 1000);
						}
						divmod(Microseconds, 1000);
					}
					divmod(Milliseconds, 1000);
				}
				divmod(Seconds, 60);
			}
			divmod(Minutes, 60);
		}
		divmod(Hours, 24);
	}

	nanoseconds = elem[Nanoseconds];
	microseconds = elem[Microseconds];
	milliseconds = elem[Milliseconds];
	seconds = elem[Seconds];
	minutes = elem[Minutes];
	hours = elem[Hours];
	days = elem[Days];
}

/**
 * @brief Class to handle a simple time value with associated unit
 */
template <typename T> struct Time {
	Time() = default;

	Time(Unit unit, T time) : unit(unit), time(time)
	{
	}

	void set(Unit unit, T time)
	{
		this->unit = unit;
		this->time = time;
	}

	operator T() const
	{
		return time;
	}

	String toString() const
	{
		String s(time);
		s += unitToString(unit);
		return s;
	}

	friend Time& operator+(Time lhs, const Time& rhs)
	{
		lhs.time += rhs.as(lhs.unit);
		return lhs;
	}

	Time& operator+=(Time<T> rhs)
	{
		time += rhs.as(unit);
		return *this;
	}

	TimeValue value() const
	{
		return TimeValue(unit, time);
	}

	template <Unit unitTo> Time as() const
	{
		return Time(unitTo, convert(time, unit, unitTo));
	}

	Time as(Unit unitTo) const
	{
		return Time(unitTo, convert(time, unit, unitTo));
	}

	Unit unit = Seconds;
	T time = 0;
};

/**
 * @brief Helper function to create a Time and deduce the type
 */
template <typename T> Time<T> time(Unit unit, T value)
{
	return Time<T>(unit, value);
}

/**
 * @brief Class to handle a tick value associated with a clock
 */
template <typename Clock_, typename T> struct Ticks {
	using Clock = Clock_;

	static constexpr Clock clock()
	{
		return Clock();
	}

	Ticks(T ticks) : ticks(ticks)
	{
	}

	operator T() const
	{
		return ticks;
	}

	String toString() const
	{
		return String(ticks);
	}

	template <Unit unit> Time<T> as()
	{
		return Time<T>(unit, Clock::template ticksToTime<unit>(ticks));
	}

	T ticks;
};

/**
 * @brief Class template to represent a fixed time value for a specific Clock
 * @tparam Clock_
 * @tparam unit_
 * @tparam time_
 * @note Includes compile-time range checking. Time is taken as reference for conversions.
 */
template <class Clock_, Unit unit_, uint64_t time_> struct TimeConst {
	using Clock = Clock_;
	using TicksPerUnit = typename Clock::template TicksPerUnit<unit_>;

	static constexpr Clock clock()
	{
		return Clock();
	}

	static constexpr Unit unit()
	{
		return unit_;
	}

	static constexpr uint64_t time()
	{
		return time_;
	}

	constexpr operator uint64_t()
	{
		return time_;
	}

	/**
	 * @brief Get ticks per unit as a Ratio object
	 * @retval BasicRatio32
	 */
	static Ratio32 ticksPerUnit()
	{
		return BasicRatio32{TicksPerUnit::num, TicksPerUnit::den};
	}

	/**
	 * @brief Return the corresponding tick value for the time interval
	 * @retval TimeType
	 */
	static constexpr uint64_t ticks()
	{
		return round(double(time_) * TicksPerUnit::num / TicksPerUnit::den);
	}

	/**
	 * @brief Use this function to perform a static (compile-time) range check against Clock maxTicks
	 * @retval TimeType
	 */
	static constexpr void check()
	{
		static_assert(ticks() <= Clock::maxTicks(), "Time exceeds clock range");
	}

	/**
	 * @brief Obtain the actual Clock time by converting tick value
	 * @retval TimeType
	 */
	static constexpr uint64_t clockTime()
	{
		return round(double(ticks()) * TicksPerUnit::den / TicksPerUnit::num);
	}

	/**
	 * @brief Obtain the time in a different set of units
	 * @tparam unit
	 * @retval TimeType
	 */
	template <Unit unit> static constexpr uint64_t as()
	{
		return convert<time_, unit_, unit>();
	}

	static TimeValue value()
	{
		return TimeValue(unit_, time_);
	}

	static TimeValue clockValue()
	{
		return TimeValue(unit_, clockTime());
	}

	static String toString()
	{
		return Time<uint64_t>(unit_, time_).toString();
	}
};

/**
 * @brief Class template representing a fixed clock tick count
 * @tparam Source_
 * @tparam ticks_
 * @note Includes compile-time range checking
 */
template <class Clock_, uint64_t ticks_> struct TicksConst {
	using Clock = Clock_;
	using TickType = uint64_t;
	using TimeType = uint64_t;

	static constexpr TickType ticks()
	{
		return ticks_;
	}

	constexpr operator TickType()
	{
		return ticks_;
	}

	/**
	 * @brief Obtain the tick count with a static range check against Clock maxTicks
	 * @retval TimeType
	 */
	static constexpr void check()
	{
		static_assert(ticks_ <= Clock::maxTicks(), "Ticks exceeds clock range");
	}

	template <Unit unit>
	using TimeConst = TimeConst<Clock, unit,
								TimeType(round(double(ticks_) * Clock::template TicksPerUnit<unit>::den /
											   Clock::template TicksPerUnit<unit>::num))>;

	/**
	 * @brief Get the time for the tick count in a specific time unit
	 * @tparam unit
	 * @retval Time<TimeType>
	 */
	template <Unit unit> static constexpr TimeConst<unit> as()
	{
		return TimeConst<unit>();
	}

	static String toString()
	{
		return String(ticks_);
	}
};

/**
 * @brief Function template to convert a time quantity from one unit to another
 * @tparam unitsFrom Units for `time` parameter
 * @tparam unitsTo Units for return value
 * @tparam TimeType Variable type to use for calculation
 * @param time The time to convert
 * @retval TimeType Converted time, returns TimeType(-1) if calculation overflowed
 */
template <Unit unitsFrom, Unit unitsTo, typename TimeType> __forceinline TimeType convert(const TimeType& time)
{
	if(unitsFrom == unitsTo) {
		return time;
	}

	using R = std::ratio_divide<UnitTickRatio<unitsTo>, UnitTickRatio<unitsFrom>>;
	return muldiv<R::num, R::den>(time);
}

/**
 * @brief Class template for accessing a Clock in specific time units
 * @tparam Clock_
 * @tparam units_
 * @tparam TimeType_ Limits range of calculations
 * @note Includes compile-time range checking. Time is taken as reference for conversions.
 */
template <class Clock_, Unit unit_, typename TimeType_> struct TimeSource : public Clock_ {
	using Clock = Clock_;
	using TimeType = TimeType_;
	using TicksPerUnit = typename Clock::template TicksPerUnit<unit_>;
	template <uint64_t time> using TimeConst = TimeConst<Clock, unit_, time>;
	template <uint64_t ticks> using TicksConst = TicksConst<Clock, ticks>;

	static constexpr Unit unit()
	{
		return unit_;
	}

	/**
	 * @brief Number of clock ticks per unit of time
	 * @retval BasicRatio32 Result as a rational fraction
	 */
	static constexpr BasicRatio32 ticksPerUnit()
	{
		return {TicksPerUnit::num, TicksPerUnit::den};
	}

	/**
	 * @brief Get the time corresponding to the maximum clock tick value
	 */
	using MaxClockTime = typename Clock::template MaxTime<unit_>;
	static constexpr MaxClockTime maxClockTime()
	{
		return MaxClockTime();
	}

	// Check against arbitrary minimum - could be 1, but is that actually useful?
	static_assert(maxClockTime() >= 5, "Time units too large for Clock ");

	/**
	 * @brief Obtain a TimeConst type representing the given time quantity
	 * @tparam time
	 * @retval TimeConst
	 * @note Use methods of TimeConst to obtain corresponding tick count, etc.
	 */
	template <uint64_t time> static constexpr TimeConst<time> timeConst()
	{
		return TimeConst<time>();
	}

	/**
	 * @brief Class template defining a fixed tick quantity
	 * @tparam ticks
	 * @retval TicksConst
	 * @note Use methods of TickConst to obtain corresponding time values, etc.
	 */
	template <uint64_t ticks> static constexpr TicksConst<ticks> ticksConst()
	{
		return TicksConst<ticks>();
	}

	/**
	 * @brief The maximum time value supported by timeToTicks without overflowing
	 * @retval TimeType Passing values larger than this to `timeToTicks()` will truncate at maximum value
	 */
	static constexpr Time<TimeType_> maxCalcTime()
	{
		return Time<TimeType_>(unit_, MuldivLimits<TicksPerUnit::num, TicksPerUnit::den, TimeType>::maxValue());
	}

	/**
	 * @brief The maximum tick value supported by ticksToTime without overflowing
	 * @retval TimeType Passing values larger than this to `ticksToTime()` will truncate at maximum value
	 */
	static constexpr Ticks<Clock_, TimeType_> maxCalcTicks()
	{
		return MuldivLimits<TicksPerUnit::den, TicksPerUnit::num, TimeType>::maxValue();
	}

	/**
	 * @brief Get the number of ticks for a given time
	 * @param time
	 * @retval TimeType Tick count, rounded to the nearest tick
	 */
	static Ticks<Clock_, TimeType_> timeToTicks(TimeType time)
	{
		return muldiv<TicksPerUnit::num, TicksPerUnit::den>(time);
	}

	/**
	 * @brief Get the number of ticks for a given time
	 * @tparam time
	 * @retval uint64_t Tick count, rounded to the nearest tick
	 */
	template <uint64_t time> static constexpr uint64_t timeToTicks()
	{
		return TimeConst<time>::ticks();
	}

	/**
	 * @brief Get the time for a given number of clock ticks
	 * @tparam ticks
	 * @retval TimeType Time count, rounded to the nearest unit
	 */
	template <uint64_t ticks> static constexpr uint64_t ticksToTime()
	{
		return TicksConst<ticks>::template as<unit>();
	}

	/**
	 * @brief Get the time for a given number of clock ticks
	 * @param ticks
	 * @retval TimeType Time count, rounded to the nearest unit
	 */
	static Time<TimeType_> ticksToTime(TimeType ticks)
	{
		return Time<TimeType_>(unit_, muldiv<TicksPerUnit::den, TicksPerUnit::num>(ticks));
	}

	static String toString()
	{
		String s;
		s += Clock::toString();
		s += '/';
		s += sizeof(TimeType) * 8;
		s += "-bit/";
		s += unitToLongString(unit_);
		return s;
	}
};

} // namespace NanoTime

/** @} */
