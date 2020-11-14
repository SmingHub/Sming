#include <HostTests.h>
#include <FlashString/TemplateStream.hpp>
#include <Data/Stream/MemoryDataStream.h>

DEFINE_FSTR_LOCAL(template1, "Stream containing {var1}, {var2} and {var3}. {} {{}} {{12345")
DEFINE_FSTR_LOCAL(template1_1, "Stream containing value #1, value #2 and {var3}. {} {{}} {{12345")
DEFINE_FSTR_LOCAL(template1_2, "Stream containing value #1, value #2 and [value #3]. {} {{}} {{12345")

DEFINE_FSTR_LOCAL(template2, "This text should {disable}not {var1} really {var2:hello} again {enable}be missing.")
DEFINE_FSTR_LOCAL(template2_1, "This text should be missing.")

class TemplateStreamTest : public TestGroup
{
public:
	TemplateStreamTest() : TestGroup(_F("Stream"))
	{
	}

	void execute() override
	{
		TEST_CASE("template1.1")
		{
			FSTR::TemplateStream tmpl(template1);
			tmpl.onGetValue([](const char* name) -> String {
				if(FS("var1") == name) {
					return F("value #1");
				}
				if(FS("var2") == name) {
					return F("value #2");
				}
				return nullptr;
			});

			check(tmpl, template1_1);
		}

		TEST_CASE("template1.2")
		{
			FSTR::TemplateStream tmpl(template1);
			tmpl.setVar("var3", "[value #3]");
			tmpl.onGetValue([](const char* name) -> String {
				if(FS("var1") == name) {
					return F("value #1");
				}
				if(FS("var2") == name) {
					return F("value #2");
				}
				return nullptr;
			});

			check(tmpl, template1_2);
		}

		TEST_CASE("template2.1")
		{
			Serial.println(template2);
			FSTR::TemplateStream tmpl(template2);
			tmpl.onGetValue([&tmpl](const char* name) -> String {
				if(FS("disable") == name) {
					tmpl.enableOutput(false);
					return "";
				}
				if(FS("enable") == name) {
					tmpl.enableOutput(true);
					return "";
				}
				return nullptr;
			});

			check(tmpl, template2_1);
		}
	}

private:
	void check(TemplateStream& stream, const FlashString& ref)
	{
		MemoryDataStream mem;
		mem.copyFrom(&stream, 1024);
		String tmpl;
		REQUIRE(mem.moveString(tmpl));
		debug_i("tmpl: %s", tmpl.c_str());
		debug_i(" ref: %s", String(ref).c_str());
		REQUIRE(ref == tmpl);
	}
};

void REGISTER_TEST(TemplateStream)
{
	registerGroup<TemplateStreamTest>();
}
