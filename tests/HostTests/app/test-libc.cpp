#include <SmingTest.h>

static int num_instances;

struct A {
	A()
	{
		order = ++num_instances;
	}

	int order;
};

namespace libc_initorder
{
A a3;
A a4;
A a2 __attribute__((init_priority(1000)));
A a1 __attribute__((init_priority(990)));
} // namespace libc_initorder

// Simple check to determine if a pointer refers to ROM
#define isRomPtr(ptr) (uint32_t(ptr) >= 0x40000000 && uint32_t(ptr) < 0x40100000)

/*
 * Anything to do with main C/C++ libraries, startup code, etc.
 */
class LibcTest : public TestGroup
{
public:
	LibcTest() : TestGroup(_F("LibC"))
	{
	}

	void execute() override
	{
		// https://github.com/esp8266/Arduino/issues/6870
		TEST_CASE("Global object init order")
		{
			using namespace libc_initorder;
			REQUIRE(a1.order == 1);
			REQUIRE(a2.order == 2);
			REQUIRE(a3.order == 3);
			REQUIRE(a4.order == 4);
		}

#ifdef ARCH_ESP8266
		TEST_CASE("functions in ROM")
		{
			REQUIRE(isRomPtr(memcpy));
			REQUIRE(isRomPtr(memmove));
			REQUIRE(isRomPtr(memset));
			REQUIRE(isRomPtr(memcmp));
			REQUIRE(isRomPtr(strlen));
			REQUIRE(isRomPtr(strcmp));
			REQUIRE(isRomPtr(strncmp));
			REQUIRE(isRomPtr(strstr));
		}
#endif
	}
};

void REGISTER_TEST(libc)
{
	registerGroup<LibcTest>();
}
