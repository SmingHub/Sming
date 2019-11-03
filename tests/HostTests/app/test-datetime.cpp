#include "common.h"

#include <DateTime.h>

class DateTimeTest : public TestGroup
{
public:
	DateTimeTest() : TestGroup(_F("DateTime"))
	{
	}

	void execute() override
	{
		DEFINE_FSTR_LOCAL(testDateString, "Sun, 06 Nov 1994 08:49:37 GMT");
		constexpr time_t testDate = 784111777;

		DateTime dt;
		REQUIRE(dt.fromHttpDate(testDateString) == true);

		debug_i("parseHttpDate(\"%s\") produced \"%s\"", String(testDateString).c_str(),
				dt.toFullDateTimeString().c_str());
		time_t unixTime = dt.toUnixTime();
		REQUIRE(unixTime == testDate);
		dt = testDate;
		debug_d("\"%s\"", dt.toFullDateTimeString().c_str());

		checkTime(59, 59, 23, 14, 2, 2019);
		checkTime(13, 1, 1, 1, 1, 1970);
	}

	void checkTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year)
	{
		DateTime dt;
		dt.setTime(sec, min, hour, day, month, year);
		debug_i("Checking '%s'", dt.toFullDateTimeString().c_str());
		time_t unixTime = dt.toUnixTime();
		dt.setTime(unixTime);
		REQUIRE(dt.Second == sec);
		REQUIRE(dt.Minute == min);
		REQUIRE(dt.Hour == hour);
		REQUIRE(dt.Day == day);
		REQUIRE(dt.Month == month);
		REQUIRE(dt.Year == year);
	}
};

void REGISTER_TEST(datetime)
{
	registerGroup<DateTimeTest>();
}
