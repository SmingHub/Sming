#include <HostTests.h>

static int num_instances;

struct A {
	A() : order(++num_instances)
	{
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
			debug_i("order: %d, %d, %d, %d", a1.order, a2.order, a3.order, a4.order);
			// Compilers may legitimately order these in two different ways
			if(a1.order == 3) {
				REQUIRE(a1.order == 3);
				REQUIRE(a2.order == 4);
				REQUIRE(a3.order == 1);
				REQUIRE(a4.order == 2);
			} else {
				REQUIRE(a1.order == 1);
				REQUIRE(a2.order == 2);
				REQUIRE(a3.order == 3);
				REQUIRE(a4.order == 4);
			}
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

		TEST_CASE("64-bit mprintf")
		{
			char buffer[256];
			m_snprintf(buffer, sizeof(buffer), "%x", 0x12345678);
			REQUIRE_EQ(String(buffer), "12345678");
			m_snprintf(buffer, sizeof(buffer), "%u", 12345678);
			REQUIRE_EQ(String(buffer), "12345678");
			m_snprintf(buffer, sizeof(buffer), "%llx", 0x123456789ABCDEFULL);
			REQUIRE_EQ(String(buffer), "123456789abcdef");
			m_snprintf(buffer, sizeof(buffer), "0x%016llX", 0x123456789ABCDEFULL);
			REQUIRE_EQ(String(buffer), "0x0123456789ABCDEF");
			m_snprintf(buffer, sizeof(buffer), "%llu", 123456789123456789ULL);
			REQUIRE_EQ(String(buffer), "123456789123456789");
		}
	}
};

void REGISTER_TEST(Libc)
{
	registerGroup<LibcTest>();
}
