#include <SmingTest.h>

#include <Data/ObjectMap.h>

static unsigned objectCount = 0;

class TestClass
{
public:
	TestClass()
	{
		++objectCount;
	}

	~TestClass()
	{
		--objectCount;
	}
};

using TestMap = ObjectMap<String, TestClass>;

class ObjectMapTest : public TestGroup
{
public:
	ObjectMapTest() : TestGroup(_F("ObjectMap"))
	{
	}

	void execute() override
	{
		TestMap map;

		TEST_CASE("Add items")
		{
			for(unsigned i = 0; i < 5; ++i) {
				String key = "Object " + String(i);
				debug_i("map[\"%s\"] = new", key.c_str());
				map[key] = new TestClass;
			}
			REQUIRE(map.count() == 5);
			REQUIRE(objectCount == 5);
		}

		TEST_CASE("Set duplicate items")
		{
			for(unsigned i = 0; i < 5; ++i) {
				String key = "Object " + String(i);
				debug_i("map[\"%s\"] = new", key.c_str());
				map[key] = new TestClass;
			}
			REQUIRE(map.count() == 5);
			REQUIRE(objectCount == 5);
		}

		TEST_CASE("Non-existent values")
		{
			TestMap::Value value = map["non existent key"];
			debug_i("map[\"%s\"] = %p", value.getKey().c_str(), (void*)value);
			REQUIRE(map.count() == 5);
			value = new TestClass;
			REQUIRE(map.count() == 6);
		}

		TEST_CASE("extract")
		{
			auto obj = map.extract("Object 0");
			REQUIRE(map.count() == 5);
			REQUIRE(objectCount == 6);
			debug_i("\"Object 0\" -> %p", obj);
			delete obj;
			REQUIRE(objectCount == 5);
		}

		TEST_CASE("Set to null")
		{
			for(unsigned i = 0; i < 5; ++i) {
				String key = "Object " + String(i);
				debug_i("map[\"%s\"] = nullptr", key.c_str());
				map[key] = nullptr;
			}
			REQUIRE(map.count() == 6);
			REQUIRE(objectCount == 1);
		}

		TEST_CASE("clear")
		{
			map.clear();
			REQUIRE(map.count() == 0);
			REQUIRE(objectCount == 0);
		}
	}
};

void REGISTER_TEST(objectmap)
{
	registerGroup<ObjectMapTest>();
}
