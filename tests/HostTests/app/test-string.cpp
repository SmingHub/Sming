#include "common.h"

#include <Data/HexString.h>

class StringTest : public TestGroup
{
public:
	StringTest() : TestGroup(_F("String"))
	{
	}

	void execute() override
	{
		testString();
		testMakeHexString();
	}

	void testString()
	{
		startTest("testString");

		DEFINE_FSTR_LOCAL(FS_Text, "The quick brown fox jumps over a lazy dog.");
		String text = FS_Text;

		auto replace = [&](const String& from, const String& to) {
			text.replace(from, to);
			debug_hex(DBG, "from", from.c_str(), from.length());
			debug_hex(DBG, "to", to.c_str(), to.length());
			debug_hex(DBG, "result", text.c_str(), text.length() + 1);
		};

		debug_hex(DBG, "Text", text.c_str(), text.length() + 1);

		DEFINE_FSTR_LOCAL(FS_Text1, "The\0!\0quick\0!\0brown\0!\0fox\0!\0jumps\0!\0over\0!\0a\0!\0lazy\0!\0dog.");
		replace(" ", F("\0!\0"));
		REQUIRE(FS_Text1 == text);

		DEFINE_FSTR_LOCAL(FS_Text2, "The!quick!brown!fox!jumps!over!a!lazy!dog.");
		replace(F("\x00"), nullptr);
		REQUIRE(FS_Text2 == text);

		replace("!", " ");
		REQUIRE(FS_Text == text);

		DEFINE_FSTR_LOCAL(FS_Text3, "The quick brown fox jumps over \x02 l\x02zy dog.");
		replace("a", "\x02");
		REQUIRE(FS_Text3 == text);

		DEFINE_FSTR_LOCAL(FS_Text4, "The quick \x00\bad\x00rown fox jumps over \x02 l\x02zy dog.");
		replace("b", F("\x00\bad\x00"));
		REQUIRE(FS_Text4 == text);

		DEFINE_FSTR_LOCAL(test, "This is a some test data \1\2\3 Not all ASCII\0"
								"\0"
								"\0"
								"Anything, really\0"
								"Can I go home now?");
		LOAD_FSTR(data, test);
		m_printHex(_F("TEST1"), data, test.length());
		m_printHex(_F("TEST2"), data, test.length(), 0);
		m_printHex(_F("TEST3"), data, test.length(), 0xFFFFF9, 8);
		m_printHex(nullptr, data, test.length(), 0x7FFFFFF9, 0);

		debugf("\nTest String concat");
		String path = "/path/to";
		String query;

		debugf("path = \"%s\"", path.c_str());
		debugf("query = \"%s\"", query.c_str());

		String tmpString = path + query;
		debugf("path + query = \"%s\"", tmpString.c_str());

		String tmpString2 = query + path;
		debugf("query + path = \"%s\"", tmpString2.c_str());
	}

	void testMakeHexString()
	{
		uint8 hwaddr[] = {0xaa, 0xbb, 0xcc, 0xdd, 0x12, 0x55, 0x00};
		REQUIRE(makeHexString(nullptr, 6) == String::empty);
		REQUIRE(makeHexString(hwaddr, 0) == String::empty);
		REQUIRE(makeHexString(hwaddr, 6) == F("aabbccdd1255"));
		REQUIRE(makeHexString(hwaddr, 6, ':') == F("aa:bb:cc:dd:12:55"));
		REQUIRE(makeHexString(hwaddr, 7) == F("aabbccdd125500"));
		REQUIRE(makeHexString(hwaddr, 7, ':') == F("aa:bb:cc:dd:12:55:00"));
		REQUIRE(makeHexString(hwaddr, 1, ':') == F("aa"));
		REQUIRE(makeHexString(hwaddr, 0, ':') == String::empty);
	}
};

void REGISTER_TEST(string)
{
	registerGroup<StringTest>();
}
