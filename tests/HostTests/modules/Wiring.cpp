#include <HostTests.h>

#include <WHashMap.h>
#include <WVector.h>
#include <MacAddress.h>
#include <Data/WebConstants.h>
#include <map>
#include <vector>
#include <malloc_count.h>

namespace
{
template <typename A, typename B> Print& operator<<(Print& p, const std::pair<A, B>& e)
{
	p << e.first << " = " << e.second;
	return p;
}

template <typename T> void print(const T& list, const char* separator = "\r\n")
{
	for(const auto& e : list) {
		Serial << e << separator;
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

} // namespace

class WiringTest : public TestGroup
{
public:
	WiringTest() : TestGroup(_F("Wiring"))
	{
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
				REQUIRE(e->startsWith("value"));
			}

			for(auto e : map) {
				*e += ": gobbed";
			}
			for(auto e : map) {
				REQUIRE(e->endsWith("gobbed"));
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
			auto time = [](const String& description, const Func& function) {
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

			print(vector);

			for(auto& e : vector) {
				e += ": gobbed";
			}
			for(auto& e : vector) {
				CHECK(e.length() == 16 && e.endsWith("gobbed"));
			}

			vector.setElementAt("potato", 1);
			REQUIRE(vector[1] == "potato");
			REQUIRE(vector.count() == 4);

			vector[1] = "cabbage";
			REQUIRE(vector[1] == "cabbage");
			REQUIRE(vector.count() == 4);

			REQUIRE(!vector.insertElementAt("radish", 5));
			REQUIRE(vector.insertElementAt("radish", 4));
			REQUIRE(vector[4] == "radish");

			REQUIRE(vector.firstElement() == "value(a): gobbed");
			REQUIRE(vector.lastElement() == "radish");

			REQUIRE(vector.remove(2));
			REQUIRE(vector[2] == "value(d): gobbed");

			REQUIRE(vector.setSize(3));
			REQUIRE_EQ(vector.count(), 3);
			REQUIRE_EQ(vector.capacity(), 14);

			vector.trimToSize();
			REQUIRE_EQ(vector.capacity(), 3);

			String arr[3];
			vector.copyInto(arr);
			for(unsigned i = 0; i < vector.count(); ++i) {
				REQUIRE_EQ(vector[i], arr[i]);
			}

			REQUIRE(vector.addElement(new String("banana")));
			REQUIRE_EQ(vector.count(), 4);
			REQUIRE_EQ(vector.capacity(), 13);
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

			print(vector);

			for(auto& e : vector) {
				e += ": gobbed";
			}

			print(vector);
		}

		TEST_CASE("Vector<uint8_t>")
		{
			auto startMem = MallocCount::getCurrent();

			Vector<uint8_t> vector(32);
			for(unsigned i = 0; i < 32; ++i) {
				vector.add(os_random());
			}

			Serial << "Heap " << MallocCount::getCurrent() - startMem << endl;

			print(vector, ",");
			Serial.println();

			for(auto& e : vector) {
				e += 12;
			}

			print(vector, ",");
			Serial.println();

			vector.setElementAt(0, 1);
			REQUIRE(vector[1] == 0);
			REQUIRE(vector.count() == 32);
			REQUIRE(vector.capacity() == 32);

			REQUIRE(!vector.insertElementAt(99, 35));
			REQUIRE(vector.insertElementAt(99, 32));
			REQUIRE(vector[32] == 99);
			REQUIRE_EQ(vector.capacity(), 42);

			REQUIRE(vector.setSize(3));
			REQUIRE_EQ(vector.count(), 3);
			REQUIRE_EQ(vector.capacity(), 42);

			vector.trimToSize();
			REQUIRE_EQ(vector.capacity(), 3);

			uint8_t arr[3];
			vector.copyInto(arr);
			for(unsigned i = 0; i < vector.count(); ++i) {
				REQUIRE_EQ(vector[i], arr[i]);
			}
		}

		TEST_CASE("std::vector<uint8_t>")
		{
			auto startMem = MallocCount::getCurrent();

			std::vector<uint8_t> vector;
			for(unsigned i = 0; i < 32; ++i) {
				vector.push_back(os_random());
			}

			Serial << "Heap " << MallocCount::getCurrent() - startMem << endl;

			for(auto& e : vector) {
				Serial << e << ", ";
			}
			Serial.println();

			for(auto& e : vector) {
				e += 12;
			}

			for(auto& e : vector) {
				Serial << e << ", ";
			}
			Serial.println();
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
