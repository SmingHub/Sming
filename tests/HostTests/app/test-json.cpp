#include "common.h"

void test_json()
{
	DEFINE_FSTR_LOCAL(flashString1, "FlashString-1");
	DEFINE_FSTR_LOCAL(flashString2, "FlashString-2");
	DEFINE_FSTR_LOCAL(formatStrings, "Compact\0Pretty\0MessagePack");
	DEFINE_FSTR_LOCAL(test_json, "test.json");
	DEFINE_FSTR_LOCAL(test_msgpack, "test.msgpack");

	spiffs_mount();

	StaticJsonDocument<512> doc;
	doc["string1"] = "string value 1";
	doc["number2"] = 12345;
	auto arr = doc.createNestedArray("arr");
	arr.add(flashString1);
	doc[flashString2] = flashString1;

	startTest("serialize");
	{
		DEFINE_FSTR_LOCAL(serialized1,
						  "{\"string1\":\"string value "
						  "1\",\"number2\":12345,\"arr\":[\"FlashString-1\"],\"FlashString-2\":\"FlashString-1\"}");

		String s = Json::serialize(doc);
		debug_i("Test doc: %s", s.c_str());
		assert(s == serialized1);
	}

	//
	startTest("Json::measure()");
	{
		CStringArray formats(formatStrings);
		const uint8_t sizes[] = {100, 132, 82};
		for(auto fmt = Json::Compact; fmt <= Json::MessagePack; ++fmt) {
			auto len = Json::measure(doc, fmt);
			debug_i("Measure(doc, %s) = %u", formats[fmt], len);
			assert(len == sizes[fmt]);
		}
	}

	//
	startTest("Json::getValue(doc[\"number2\"], value))");
	{
		int value;
		if(Json::getValue(doc["number2"], value)) {
			debug_i("number2 = %d", value);
			assert(value == 12345);
		} else {
			debug_e("number2 not found");
		}
	}

	//
	startTest("Json::getValue(doc[\"string\"], value))");
	{
		String value;
		if(Json::getValue(doc["string"], value)) {
			debug_i("string = %d", value.c_str());
			assert(false);
		} else {
			debug_e("string not found");
		}
	}

	//
	startTest("Json::getValue(doc[\"arr\"][1], value");
	{
		String value;
		if(Json::getValue(doc["arr"][0], value)) {
			debug_i("arr = %s", value.c_str());
			assert(value == flashString1);
		} else {
			debug_e("arr not found");
			assert(false);
		}
	}

	// Keep a reference copy for when doc gets messed up
	StaticJsonDocument<512> sourceDoc = doc;

	//
	startTest("Json::serialize(doc, String), then save to file");
	{
		String s;
		Json::serialize(doc, s);
		fileSetContent(test_json, s);
	}

	//
	startTest("Json::saveToFile(doc, test_json, Json::Pretty)");
	bool res = Json::saveToFile(doc, test_json, Json::Pretty);
	debug_i("writeToFile %s", res ? "OK" : "FAILED");
	assert(res);

	//
	startTest("Json::loadFromFile(doc, test_json)");
	{
		res = Json::loadFromFile(doc, test_json);
		debug_i("loadFromFile %s", res ? "OK" : "FAILED");
		assert(res);
		String s = fileGetContent(test_json);
		Serial.println(s);
	}

	//
	startTest("Json::serialize(doc, MemoryDataStream*)");
	{
		doc = sourceDoc;
		auto stream = new MemoryDataStream;
		Json::serialize(doc, stream, Json::Compact);
		auto avail = stream->available();
		debug_i("serialize -> %d bytes", avail);
		assert(avail == 100);
		Json::deserialize(doc, stream, Json::Compact);
		auto measured = Json::measure(doc);
		debug_i("deserialize -> %u bytes", measured);
		assert(measured == size_t(avail));
		Serial.println(Json::serialize(doc));
		delete stream;
	}

	//
	startTest("nullptr checks");
	{
		MemoryDataStream* stream = nullptr;
		auto count = Json::serialize(doc, stream);
		debug_i("Json::serialize(stream = nullptr) = %u", count);
		auto err = Json::deserialize(doc, stream);
		debug_i("Json::deserialize(stream = nullptr) = %u", err);
		debug_i("doc.memoryUsage = %u", doc.memoryUsage());
	}

	//
	String serialised;
	startTest("String serialisation");
	{
		doc = sourceDoc;
		serialised = Json::serialize(doc);
		m_printHex("serialized", serialised.c_str(), serialised.length());
		String s;
		Json::deserialize(doc, s);
		m_printHex("de-serialized", s.c_str(), s.length());
		debug_i("doc.memoryUsage = %u", doc.memoryUsage());
	}

	//
	startTest("Buffer serialisation");
	char buffer[256];
	{
		doc = sourceDoc;
		size_t len = Json::serialize(doc, buffer);
		m_printHex("Serialised", buffer, len);
		assert(len == serialised.length());
		assert(memcmp(buffer, serialised.c_str(), len) == 0);
	}

	//
	startTest("Buffer/Size serialisation");
	{
		doc = sourceDoc;
		auto len = Json::serialize(doc, buffer, sizeof(buffer));
		m_printHex("Serialised", buffer, len);
		assert(len == serialised.length());
		assert(memcmp(buffer, serialised.c_str(), len) == 0);
		Json::deserialize(doc, buffer, len);
		m_printHex("De-serialized", buffer, len);
		debug_i("doc.memoryUsage = %u", doc.memoryUsage());
	}

	//
	startTest("Json::saveToFile(doc, test_msgpack, Json::MessagePack)");
	{
		res = Json::saveToFile(doc, test_msgpack, Json::MessagePack);
		debug_i("writeToFile(%s, MessagePack)", res ? "OK" : "FAILED");
	}

	//
	startTest("Json::loadFromFile(doc, test_msgpack, Json::MessagePack)");
	{
		res = Json::loadFromFile(doc, test_msgpack, Json::MessagePack);
		debug_i("MsgPack::loadFromFile %s", res ? "OK" : "FAILED");
		assert(res);
		String s = fileGetContent(test_msgpack);
		m_printHex("MSG", s.c_str(), s.length());
		assert(s.length() == 82);
	}

	//
	startTest("Json::serialize(doc, Serial)");
	{
		Json::serialize(doc, Serial);
		Serial.println();
		Json::serialize(doc, Serial, Json::Pretty);
		Serial.println();
	}

	//
	startTest("Json::measure(doc2)");
	{
		StaticJsonDocument<10> doc2;
		auto measured = Json::measure(doc2);
		debug_i("measure(doc2) = %u", measured);
		assert(measured == 4);
		String s;
		Json::serialize(doc2, s);
		m_printHex("doc2", s.c_str(), s.length());
		assert(s == "null");
	}

	// Serialization
	startTest("Serialise to MemoryDataStream");
	const char jsonTest[] = "{\"key1\":\"value1\",\"key2\":\"value2\"}";
	Json::deserialize(doc, jsonTest);
	{
		auto stream = new MemoryDataStream;
		stream->setTimeout(0);
		size_t serializedLength = Json::serialize(doc, stream);
		debug_i("serialized length = %u", serializedLength);
		assert(serializedLength == 33);
		String content = stream->readString();
		debug_i("stream->read returned %u", content.length());
		Serial.println(content);
		assert(content == jsonTest);
		delete stream;
	}

	// De-serialisation
	{
		startTest("De-serialise from MemoryDataStream");
		auto stream = new MemoryDataStream;
		stream->write(reinterpret_cast<const uint8_t*>(jsonTest), sizeof(jsonTest));
		bool res = Json::deserialize(doc, stream);
		debug_i("Json::deserialize() returned %u", res);
		assert(res == true);
		String s;
		serializeJson(doc, s);
		Serial.println(s);
		assert(s == jsonTest);
		delete stream;
	}
}
