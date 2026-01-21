#include <HostTests.h>
#include <Data/Range.h>

class RangeTest : public TestGroup
{
public:
	RangeTest() : TestGroup(_F("Range"))
	{
	}

	void execute() override
	{
		TEST_CASE("constexpr")
		{
			constexpr TRange<int> range(0, 100);
			constexpr auto int64 = 120000000000LL;
			constexpr auto val = range.clip(int64);
			static_assert(val == 100, "Bad clip");

			constexpr int64_t tmp = 0x8000000000LL;
			static_assert(!range.contains(tmp));
		}

		TEST_CASE("truncation")
		{
			constexpr TRange<int8_t> range(0, 100);
			int val = 0x1020;
			val = range.clip(val);
			REQUIRE_EQ(val, 100);
		}

		TEST_CASE("Membership")
		{
			constexpr TRange<int8_t> range(0, 100);
			int val = 0x8000;
			REQUIRE(!range.contains(val));
		}
	}
};

void REGISTER_TEST(Range)
{
	registerGroup<RangeTest>();
}
