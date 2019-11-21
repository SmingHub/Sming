#include <SmingTest.h>

#include <iram_precache.h>

class PrecacheTest : public TestGroup
{
public:
	PrecacheTest() : TestGroup(_F("IRAM Pre-caching"))
	{
	}

	void execute() override
	{
		testPrecache();
	}

	void testPrecache() IRAM_PRECACHE_ATTR
	{
		IRAM_PRECACHE_START(hosttests_test);
		unsigned sum = 0;
		for(unsigned i = 0; i < 1000; ++i) {
			sum += i;
		}
		IRAM_PRECACHE_END(hosttests_test);
		debugf("Sum = %u", sum);
		REQUIRE(sum == 499500);
	}
};

void REGISTER_TEST(precache)
{
	registerGroup<PrecacheTest>();
}
