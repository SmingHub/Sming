#include <HostTests.h>
#include <Data/Format/Json.h>

class FormatterTest : public TestGroup
{
public:
	FormatterTest() : TestGroup(_F("Formatter"))
	{
	}

	void execute() override
	{
		// Note: \xa3 is unicode for £
		DEFINE_FSTR_LOCAL(text1, "A JSON\ntest string\twith escapes\x12\0\n"
								 "Worth \"maybe\" \xa3 0.53. Yen \xa5 5bn.")

		TEST_CASE("JSON")
		{
			DEFINE_FSTR_LOCAL(text1b, "A JSON\\ntest string\\twith escapes\\u0012\\u0000\\n"
									  "Worth \\\"maybe\\\" \\u00a3 0.53. Yen \\u00a5 5bn.")

			Serial << text1 << endl;
			String s(text1);
			Format::json.escape(s);
			REQUIRE_EQ(s, text1b);

			s = text1;
			Format::json.quote(s);
			String quoted = String('"') + text1b + '"';
			REQUIRE_EQ(s, quoted);
		}

		TEST_CASE("C++")
		{
			DEFINE_FSTR_LOCAL(
				text1a, "A JSON\\ntest string\\twith escapes\\x12\\0\\nWorth \\\"maybe\\\" \xa3 0.53. Yen \xa5 5bn.")
			String s(text1);
			Format::escapeControls(s, Format::Option::doublequote | Format::Option::backslash);
			REQUIRE_EQ(s, text1a);

			DEFINE_FSTR_LOCAL(
				text1b,
				"A JSON\\ntest string\\twith escapes\\x12\\0\\nWorth \\\"maybe\\\" \xc2\xa3 0.53. Yen \xc2\xa5 5bn.")
			s = text1;
			Format::escapeControls(s, Format::Option::utf8 | Format::Option::doublequote | Format::Option::backslash);
			REQUIRE_EQ(s, text1b);
		}
	}
};

void REGISTER_TEST(Formatter)
{
	registerGroup<FormatterTest>();
}
