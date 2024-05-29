#include <HostTests.h>

#include <Data/HexString.h>

class StringTest : public TestGroup
{
public:
	StringTest() : TestGroup(_F("String"))
	{
	}

	void execute() override
	{
		templateTest(12);
		nonTemplateTest();

		testString();
		testMove();
		testMakeHexString();
	}

	template <typename T> void templateTest(T x)
	{
#ifndef ARCH_HOST
		auto pstr = PSTR("This PSTR should get moved out of RAM, filtered by __pstr__ in symbol name.");
		REQUIRE(isFlashPtr(pstr));

		auto fstr1 = FS_PTR("This FSTR should get moved out of RAM, filtered by __fstr__ in symbol name.");
		REQUIRE(isFlashPtr(fstr1));

		DEFINE_FSTR_LOCAL(fstr2, "Regular FSTR");
		REQUIRE(isFlashPtr(&fstr2));

		auto func = __PRETTY_FUNCTION__;
		REQUIRE(isFlashPtr(func));
		debug_i("%s", func);
#endif
	}

	void nonTemplateTest()
	{
#ifndef ARCH_HOST
		auto pstr = PSTR("This PSTR should get moved out of RAM, filtered by section name.");
		REQUIRE(isFlashPtr(pstr));

		auto fstr1 = FS_PTR("This FSTR should get moved out of RAM, filtered by section name.");
		REQUIRE(isFlashPtr(fstr1));

		DEFINE_FSTR_LOCAL(fstr2, "Regular FSTR");
		REQUIRE(isFlashPtr(&fstr2));

		auto func = __PRETTY_FUNCTION__;
		REQUIRE(isFlashPtr(func));
		debug_i("%s", func);
#endif
	}

	void testString()
	{
		DEFINE_FSTR_LOCAL(FS_Text, "The quick brown fox jumps over a lazy dog.");
		String text = FS_Text;

		auto replace = [&](const String& from, const String& to) {
			text.replace(from, to);
			debug_hex(DBG, "from", from.c_str(), from.length());
			debug_hex(DBG, "to", to.c_str(), to.length());
			debug_hex(DBG, "result", text.c_str(), text.length() + 1);
		};

		debug_hex(DBG, "Text", text.c_str(), text.length() + 1);

		TEST_CASE("replace with NULs, increase length")
		{
			DEFINE_FSTR_LOCAL(FS_Text1, "The\0!\0quick\0!\0brown\0!\0fox\0!\0jumps\0!\0over\0!\0a\0!\0lazy\0!\0dog.");
			replace(" ", F("\0!\0"));
			REQUIRE(FS_Text1 == text);
		}

		TEST_CASE("replace with NULs, reduce length")
		{
			DEFINE_FSTR_LOCAL(FS_Text2, "The!quick!brown!fox!jumps!over!a!lazy!dog.");
			replace(F("\x00"), nullptr);
			REQUIRE(FS_Text2 == text);
		}

		TEST_CASE("replace, no length change")
		{
			replace("!", " ");
			REQUIRE(FS_Text == text);
		}

		TEST_CASE("replace, non-NUL, non-printable value, no length change")
		{
			DEFINE_FSTR_LOCAL(FS_Text3, "The quick brown fox jumps over \x02 l\x02zy dog.");
			replace("a", "\x02");
			REQUIRE(FS_Text3 == text);
		}

		TEST_CASE("replace, increase length")
		{
			DEFINE_FSTR_LOCAL(FS_Text4, "The quick \x00\bad\x00rown fox jumps over \x02 l\x02zy dog.");
			replace("b", F("\x00\bad\x00"));
			REQUIRE(FS_Text4 == text);
		}

		TEST_CASE("replace, increase length with first char (Bug 24/10/2021)")
		{
			String s = F("abcdefa");
			s.replace("a", "aa");
			REQUIRE(F("aabcdefaa") == s);
		}

		TEST_CASE("content check (manual inspection)")
		{
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
		}

		String path = "/path/to";
		String query;

		Serial << _F("path = \"") << path << '"' << endl;
		Serial << _F("query = \"") << query << '"' << endl;

		TEST_CASE("validity check")
		{
			REQUIRE(path);
		}

		TEST_CASE("nullstr check")
		{
			REQUIRE(!query);
		}

		TEST_CASE("string + nullstr")
		{
			String s = path + query;
			Serial << _F("path + query = \"") << s << '"' << endl;
			REQUIRE(s == path);
		}

		TEST_CASE("nullstr + string")
		{
			String s = query + path;
			Serial << _F("query + path = \"") << s << '"' << endl;
			REQUIRE(s == path);
		}
	}

	void testMove()
	{
		DEFINE_FSTR_LOCAL(shortText, "Not long")
		DEFINE_FSTR_LOCAL(longText, "Greater than SSO buffer length")

		TEST_CASE("Move into unassigned string")
		{
			// Normal move
			String s1 = longText;
			String s2 = std::move(s1);
			REQUIRE(!s1);
			REQUIRE(s2.length() == longText.length());
		}

		TEST_CASE("Move between allocated strings of same length")
		{
			String s1 = longText;
			auto cstrWant = s1.c_str();
			String s2 = std::move(s1);
			REQUIRE(s2.c_str() == cstrWant);
		}

		TEST_CASE("Move to allocated string of shorter length")
		{
			String s1 = longText;
			String s2 = shortText;
			auto cstrWant = s1.c_str();
			s2 = std::move(s1);
			REQUIRE(s2.c_str() == cstrWant);
		}

		TEST_CASE("Move to allocated string of longer length")
		{
			String s1 = longText;
			String s2;
			auto cstrWant = s1.c_str();
			s1 = ""; // Buffer remains allocated
			s2 = std::move(s1);
			REQUIRE(s2.c_str() == cstrWant);
		}
	}

	void testMakeHexString()
	{
		TEST_CASE("makeHexString")
		{
			uint8_t hwaddr[] = {0xaa, 0xbb, 0xcc, 0xdd, 0x12, 0x55, 0x00};
			REQUIRE(makeHexString(nullptr, 6) == String::empty);
			REQUIRE(makeHexString(hwaddr, 0) == String::empty);
			REQUIRE(makeHexString(hwaddr, 6) == F("aabbccdd1255"));
			REQUIRE(makeHexString(hwaddr, 6, ':') == F("aa:bb:cc:dd:12:55"));
			REQUIRE(makeHexString(hwaddr, 7) == F("aabbccdd125500"));
			REQUIRE(makeHexString(hwaddr, 7, ':') == F("aa:bb:cc:dd:12:55:00"));
			REQUIRE(makeHexString(hwaddr, 1, ':') == F("aa"));
			REQUIRE(makeHexString(hwaddr, 0, ':') == String::empty);
		}
	}
};

void REGISTER_TEST(String)
{
	registerGroup<StringTest>();
}
