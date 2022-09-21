#include <HostTests.h>

#include <WHashMap.h>
#include <WVector.h>
#include <MacAddress.h>
#include <Data/WebConstants.h>
#include <map>
#include <vector>
#include <malloc_count.h>

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

	template <typename A, typename B> void println(const std::pair<A, B>& e) const
	{
		Serial.print(e.first);
		Serial.print(" = ");
		Serial.println(e.second);
	}

	template <typename A, typename B> void print(const std::map<A, B>& map) const
	{
		for(auto e : map) {
			println(e);
		}
	}

	template <typename T> void fillMap(T& map)
	{
		auto startMem = MallocCount::getCurrent();
		map[MIME_HTML] = os_random() % 0xffff;
		map[MIME_TEXT] = os_random() % 0xffff;
		map[MIME_JS] = os_random() % 0xffff;
		map[MIME_CSS] = os_random() % 0xffff;
		map[MIME_XML] = os_random() % 0xffff;
		map[MIME_JSON] = os_random() % 0xffff;
		map[MIME_JPEG] = os_random() % 0xffff;
		map[MIME_GIF] = os_random() % 0xffff;
		map[MIME_PNG] = os_random() % 0xffff;
		map[MIME_SVG] = os_random() % 0xffff;
		map[MIME_ICO] = os_random() % 0xffff;
		map[MIME_GZIP] = os_random() % 0xffff;
		map[MIME_ZIP] = os_random() % 0xffff;
		Serial << "fillMap heap " << MallocCount::getCurrent() - startMem << endl;
	}

	void execute() override
	{
		TEST_CASE("HashMap<String, String>")
		{
			auto startMem = MallocCount::getCurrent();

			HashMap<String, String> map;
			map["a"] = "value(a)";
			map["b"] = "value(b)";
			map["c"] = "value(c)";
			map["d"] = "value(d)";

			Serial << "Heap " << MallocCount::getCurrent() - startMem << endl;

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

		TEST_CASE("HashMap<MimeType, size_t>")
		{
			using TestMap = HashMap<MimeType, uint16_t>;
			TestMap map;
			fillMap(map);
			print(map);

			Serial.println();

			using Func = Delegate<void(TestMap & map)>;
			auto time = [this](const String& description, Func function) {
				Serial << description << " ..." << endl;
				TestMap map;
				fillMap(map);
				CpuCycleTimer timer;
				function(map);
				auto elapsed = timer.elapsedTime();
				print(map);
				Serial << "... took " << elapsed.toString() << endl << endl;
			};

			time("sort by key String", [](auto& map) {
				map.sort([](const auto& e1, const auto& e2) { return toString(e1.key()) < toString(e2.key()); });
			});

			time("Sort by key numerically",
				 [](auto& map) { map.sort([](const auto& e1, const auto& e2) { return e1.key() < e2.key(); }); });

			time("Sort by value",
				 [](auto& map) { map.sort([](const auto& e1, const auto& e2) { return e1.value() < e2.value(); }); });
		}

		TEST_CASE("std::map<MimeType, size_t>")
		{
			std::map<MimeType, uint16_t> map;
			fillMap(map);
			print(map);
		}

		TEST_CASE("Vector<String>")
		{
			auto startMem = MallocCount::getCurrent();

			Vector<String> vector(4);
			vector.add("value(a)");
			vector.add("value(b)");
			vector.add("value(c)");
			vector.add("value(d)");

			Serial << "Heap " << MallocCount::getCurrent() - startMem << endl;

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

		TEST_CASE("std::vector<String>")
		{
			auto startMem = MallocCount::getCurrent();

			std::vector<String> vector;
			vector.push_back("value(a)");
			vector.push_back("value(b)");
			vector.push_back("value(c)");
			vector.push_back("value(d)");

			Serial << "Heap " << MallocCount::getCurrent() - startMem << endl;

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
