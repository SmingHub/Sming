#include <SmingTest.h>

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

		TEST_CASE("fromHttpDate()")
		{
			REQUIRE(dt.fromHttpDate(testDateString) == true);
		}

		TEST_CASE("toUnixTime()")
		{
			debug_i("parseHttpDate(\"%s\") produced \"%s\"", String(testDateString).c_str(),
					dt.toFullDateTimeString().c_str());
			time_t unixTime = dt.toUnixTime();
			REQUIRE(unixTime == testDate);
		}

		dt = testDate;
		debug_d("\"%s\"", dt.toFullDateTimeString().c_str());

		TEST_CASE("setTime")
		{
			checkSetTime(59, 59, 23, 14, 2, 2019);
			checkSetTime(13, 1, 1, 1, 1, 1970);
		}
	}

	void checkSetTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year)
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
