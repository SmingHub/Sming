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
		DEFINE_FSTR_LOCAL(text1, "A JSON\ntest string\twith escapes\x12\0\n"
								 "Worth \"maybe\" \xc2\xa3"
								 "0.53.")

		TEST_CASE("JSON")
		{
			DEFINE_FSTR_LOCAL(text1b, "A JSON\\ntest string\\twith escapes\\u0012\\u0000\\n"
									  "Worth \\\"maybe\\\" \xc2\xa3"
									  "0.53.")

			Serial << text1 << endl;
			String s(text1);
			Format::json.escape(s);
			REQUIRE_EQ(s, text1b);
		}

		TEST_CASE("C++")
		{
			DEFINE_FSTR_LOCAL(text1b, "A JSON\\ntest string\\twith escapes\\x12\\0\\nWorth \\\"maybe\\\" £0.53.")

			String s(text1);
			Format::escapeControls(s, Format::Option::doublequote | Format::Option::backslash);
			REQUIRE_EQ(s, text1b);
		}
	}
};

void REGISTER_TEST(Formatter)
{
	registerGroup<FormatterTest>();
}
