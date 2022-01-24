#include <HostTests.h>

#include <WHashMap.h>
#include <WVector.h>

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
	}
};

void REGISTER_TEST(Wiring)
{
	registerGroup<WiringTest>();
}
