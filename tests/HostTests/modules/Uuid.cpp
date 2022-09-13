#include <HostTests.h>

#include <Data/Uuid.h>
#include <Data/Stream/MemoryDataStream.h>

namespace
{
struct guid_t {
	uint8_t b[16];
};

#define DEFINE_GUID(name, a, b, c, d...)                                                                               \
	static const guid_t name PROGMEM = {{(a)&0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, ((a) >> 24) & 0xff,          \
										 (b)&0xff, ((b) >> 8) & 0xff, (c)&0xff, ((c) >> 8) & 0xff, d}};

DEFINE_GUID(PARTITION_SYSTEM_GUID, 0xc12a7328, 0xf81f, 0x11d2, 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b)
#define PARTITION_SYSTEM_GUID_PSTR "c12a7328-f81f-11d2-ba4b-00a0c93ec93b"
DEFINE_FSTR_LOCAL(PARTITION_SYSTEM_GUID_FSTR, PARTITION_SYSTEM_GUID_PSTR)

} // namespace

class UuidTest : public TestGroup
{
public:
	UuidTest() : TestGroup(_F("UUID"))
	{
	}

	void execute() override
	{
		TEST_CASE("NULL GUID")
		{
			Uuid uuid;
			uint8_t empty[16]{};
			REQUIRE(memcmp(&uuid, empty, 16) == 0);
			REQUIRE(uuid == Uuid(empty));
		}

		TEST_CASE("Struct")
		{
			REQUIRE_EQ(String(PARTITION_SYSTEM_GUID_FSTR), Uuid(PARTITION_SYSTEM_GUID));
		}

		TEST_CASE("Decomposition")
		{
			REQUIRE_EQ(String(PARTITION_SYSTEM_GUID_FSTR), Uuid(PARTITION_SYSTEM_GUID_PSTR));
			REQUIRE_EQ(String(PARTITION_SYSTEM_GUID_FSTR), Uuid(PARTITION_SYSTEM_GUID_FSTR));
		}

		TEST_CASE("Copy")
		{
			Uuid u1;
			Uuid u2(PARTITION_SYSTEM_GUID);
			u1 = u2;
			REQUIRE_EQ(u1, u2);
		}

		TEST_CASE("Printing")
		{
			MemoryDataStream str;
			Uuid u1(PARTITION_SYSTEM_GUID);
			str << u1;
			String s = str.readString(Uuid::stringSize);
			REQUIRE_EQ(str.available(), 0);
			REQUIRE_EQ(s, u1);
		}
	}
};

void REGISTER_TEST(Uuid)
{
	registerGroup<UuidTest>();
}
