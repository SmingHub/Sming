#include <HostTests.h>
#include <FlashString/TemplateStream.hpp>
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Stream/LimitedMemoryStream.h>
#include <Data/Stream/SectionTemplate.h>

#ifdef ARCH_HOST
#include <IFS/Host/FileSystem.h>
#include <Data/Stream/HostFileStream.h>
#endif

DEFINE_FSTR_LOCAL(template1, "Stream containing {var1}, {var2} and {var3}. {} {{}} {{12345")
DEFINE_FSTR_LOCAL(template1_1, "Stream containing value #1, value #2 and {var3}. {} {{}} {{12345")
DEFINE_FSTR_LOCAL(template1_2, "Stream containing value #1, value #2 and [value #3]. {} {{}} {{12345")

DEFINE_FSTR_LOCAL(template2, "This text should {disable}not {var1} really {var2:hello} again {enable}be missing.")
DEFINE_FSTR_LOCAL(template2_1, "This text should be missing.")

class TemplateStreamTest : public TestGroup
{
public:
	TemplateStreamTest() : TestGroup(_F("TemplateStream"))
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

		TEST_CASE("ut_template1")
		{
			SectionTemplate tmpl(new FlashMemoryStream(Resource::ut_template1_in_rst));
			REQUIRE(tmpl.sectionCount() == 1);
			tmpl.setDoubleBraces(true);
			tmpl.onGetValue([&tmpl](const char* name) -> String {
				debug_e("getValue(%s)", name);
				if(FS("emit_contents") == name) {
					return "1";
				}

				if(memcmp(name, "emit_", 5) == 0) {
					return "";
				}

				return nullptr;
			});

#ifdef ARCH_HOST
			{
				HostFileStream fs("test-src1.out", eFO_CreateNewAlways | eFO_WriteOnly);
				int res = fs.copyFrom(&tmpl);
				debug_e("copyfrom(src) = %d", res);
				tmpl.gotoSection(0);
			}

			{
				HostFileStream fs("test-src2.out", eFO_CreateNewAlways | eFO_WriteOnly);
				int res = fs.copyFrom(&tmpl);
				debug_e("copyfrom(src) = %d", res);
				tmpl.gotoSection(0);
			}
#endif

			check(tmpl, Resource::ut_template1_out1_rst);
		}

		auto addChar = [](String& s, char c, size_t count) {
			auto len = s.length();
			s.setLength(len + count);
			memset(&s[len], c, count);
		};

		TEST_CASE("Fragmented read of variable [TMPL #1, #3, #4]")
		{
			constexpr size_t TEMPLATE_BUFFER_SIZE{100};
			String input;
			addChar(input, 'a', TEMPLATE_BUFFER_SIZE - 4);
			input += _F("{varname}");
			addChar(input, 'a', TEMPLATE_BUFFER_SIZE);
			auto source = new LimitedMemoryStream(input.begin(), input.length(), input.length(), false);
			TemplateStream tmpl(source);
			PSTR_ARRAY(someValue, "Some value or other");
			tmpl.setVar(F("varname"), someValue);

			size_t outlen{0};
			char output[TEMPLATE_BUFFER_SIZE * 3]{};
			while(!tmpl.isFinished()) {
				auto ptr = output + outlen;
				size_t read1 = tmpl.readMemoryBlock(ptr, TEMPLATE_BUFFER_SIZE);
				char tmp[read1];
				memcpy(tmp, ptr, read1);
				size_t read = tmpl.readMemoryBlock(ptr, TEMPLATE_BUFFER_SIZE);
				CHECK_EQ(read, read1);
				CHECK(memcmp(tmp, ptr, read) == 0);
				if(read > 10) {
					read -= 5;
				}
				tmpl.seek(read);
				outlen += read;
				ptr += read;
			}

			String expected;
			addChar(expected, 'a', TEMPLATE_BUFFER_SIZE - 4);
			expected += someValue;
			addChar(expected, 'a', TEMPLATE_BUFFER_SIZE);

			if(!expected.equals(output, outlen)) {
				m_nputs(output, outlen);
				m_puts("\r\n");
				m_nputs(expected.c_str(), expected.length());
				m_puts("\r\n");
			}
			REQUIRE(expected.equals(output, outlen));
		}
	}

private:
	void check(TemplateStream& tmpl, const FlashString& ref)
	{
		constexpr size_t bufSize{256};
		char buf1[bufSize];
		char buf2[bufSize];
		FSTR::Stream refStream(ref);
		while(!tmpl.isFinished()) {
			auto count = tmpl.readBytes(buf1, bufSize);
			if(count == 0) {
				continue;
			}
			if(refStream.readBytes(buf2, count) != count) {
				debug_e("TemplateStream output larger than expected");
				TEST_ASSERT(false);
			}
			if(memcmp(buf1, buf2, count) != 0) {
				debug_e("TemplateStream output incorrect");
				TEST_ASSERT(false);
			}
		}
		REQUIRE(refStream.isFinished());
	}
};

void REGISTER_TEST(TemplateStream)
{
	registerGroup<TemplateStreamTest>();
}
