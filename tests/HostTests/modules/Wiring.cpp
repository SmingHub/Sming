#include <HostTests.h>

#include <WHashMap.h>
#include <WVector.h>
#include <MacAddress.h>

class WiringTest : public TestGroup
{
public:
	WiringTest() : TestGroup(_F("Wiring"))
	{
	}

	template <typename E> void println(const E& e) const
	{
		Serial.print(e.key());
		Serial.print(" = ");
		Serial.println(*e);
	}

	template <typename Map> void print(const Map& map) const
	{
		for(auto e : map) {
			println(e);
		}
	}

	void execute() override
	{
		TEST_CASE("HashMap(String, String)")
		{
			HashMap<String, String> map;
			map["a"] = "value(a)";
			map["b"] = "value(b)";
			map["c"] = "value(c)";
			map["d"] = "value(d)";

			print(map);

			for(auto e : map) {
				String s = *e;
				e->length();
			}

			for(auto e : map) {
				*e += ": gobbed";
			}

			REQUIRE_EQ(map["b"], "value(b): gobbed");

			print(map);
		}

		TEST_CASE("Vector(String)")
		{
			Vector<String> vector;
			vector.add("value(a)");
			vector.add("value(b)");
			vector.add("value(c)");
			vector.add("value(d)");

			for(auto& e : vector) {
				Serial.println(e);
			}

			for(auto& e : vector) {
				e += ": gobbed";
			}

			for(auto& e : vector) {
				Serial.println(e);
			}
		}

		TEST_CASE("MacAddress")
		{
			const uint8_t refOctets[]{0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
			const MacAddress ref{refOctets};
			REQUIRE(memcmp(&ref[0], refOctets, 6) == 0);
			REQUIRE(MacAddress("123456789abc") == ref);
			REQUIRE(MacAddress("12:34:56:78:9A:bc") == ref);
			REQUIRE(MacAddress("12.34:56:78:9a:Bc") == ref);
			REQUIRE(MacAddress("12 34 56 78 9a bC") == ref);
			REQUIRE(MacAddress("ffffffffffff") == MacAddress({0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));
			REQUIRE(!MacAddress("123456789abc."));
			REQUIRE(!MacAddress(""));
			REQUIRE(!MacAddress("fffffffgfffff"));
		}
	}
};

void REGISTER_TEST(Wiring)
{
	registerGroup<WiringTest>();
}
