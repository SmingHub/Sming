#include <HostTests.h>
#include <FlashString/TemplateStream.hpp>
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Stream/LimitedMemoryStream.h>
#include <Data/Stream/SectionTemplate.h>
#include <Data/CsvReader.h>

#ifdef ARCH_HOST
#include <IFS/Host/FileSystem.h>
#include <Data/Stream/HostFileStream.h>
#endif

DEFINE_FSTR_LOCAL(template1, "Stream containing {var1}, {var2} and {var3}. {} {{}} {{12345")
DEFINE_FSTR_LOCAL(template1_1, "Stream containing value #1, value #2 and {var3}. {} {{}} {{12345")
DEFINE_FSTR_LOCAL(template1_2, "Stream containing value #1, value #2 and [value #3]. {} {{}} {{12345")

DEFINE_FSTR_LOCAL(template2, "This text should {disable}not {var1} really {var2:hello} again {enable}be missing.")
DEFINE_FSTR_LOCAL(template2_1, "This text should be missing.")

DEFINE_FSTR_LOCAL(template3, "<html><head><title>{title}</title><style>td { padding: 0 10px; }")
DEFINE_FSTR_LOCAL(template3_1, "<html><head><title>Document Title</title><style>td { padding: 0 10px; }")

DEFINE_FSTR_LOCAL(template4, "{\"value\":12,\"var1\":\"{var1}\"}")
DEFINE_FSTR_LOCAL(template4_1, "{\"value\":12,\"var1\":\"quoted variable\"}")

DEFINE_FSTR_LOCAL(
	test1_csv, "\"field1\",field2,field3,\"field four\"\n"
			   "Something \"awry\",\"datavalue 2\",\"where,are,\"\"the,bananas\",sausages abound,\"never surrender\"")
DEFINE_FSTR_LOCAL(csv_headings, "field1;field2;field3;field four;")
DEFINE_FSTR_LOCAL(csv_row1, "Something \"awry\";datavalue 2;where,are,\"the,bananas;sausages abound;never surrender;")

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

			check(tmpl, template1, template1_1);
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

			check(tmpl, template1, template1_2);
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

			check(tmpl, template2, template2_1);
		}

		TEST_CASE("template3 (PR #2400 - HTML)")
		{
			FSTR::TemplateStream tmpl(template3);
			tmpl.setVar("title", "Document Title");
			check(tmpl, template3, template3_1);
		}

		TEST_CASE("template4 (PR #2400 - JSON)")
		{
			FSTR::TemplateStream tmpl(template4);
			tmpl.setVar("var1", "quoted variable");
			check(tmpl, template4, template4_1);
		}

		TEST_CASE("ut_template1")
		{
			SectionTemplate tmpl(new FlashMemoryStream(Resource::ut_template1_in_rst));
			REQUIRE(tmpl.sectionCount() == 1);
			tmpl.setDoubleBraces(true);
			tmpl.onGetValue([](const char* name) -> String {
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
				HostFileStream fs("test-src1.out", File::CreateNewAlways | File::WriteOnly);
				int res = fs.copyFrom(&tmpl);
				Serial << _F("copyfrom(src) = ") << res << endl;
				tmpl.gotoSection(0);
			}

			{
				HostFileStream fs("test-src2.out", File::CreateNewAlways | File::WriteOnly);
				int res = fs.copyFrom(&tmpl);
				Serial << _F("copyfrom(src) = ") << res << endl;
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

		TEST_CASE("CSV Reader")
		{
			auto str = [](const CStringArray& cs) {
				String s = reinterpret_cast<const String&>(cs);
				s.replace('\0', ';');
				return s;
			};

			CsvReader reader(new FSTR::Stream(test1_csv));
			String headings = str(reader.getHeadings());
			Serial.println(headings);
			CHECK(reader.next());
			String row1 = str(reader.getRow());
			Serial.println(row1);
			CHECK(!reader.next());

			CHECK(csv_headings == headings);
			CHECK(csv_row1 == row1);
		}
	}

private:
	void check(TemplateStream& stream, const FlashString& tmpl, const FlashString& ref)
	{
		constexpr size_t maxLen{256};
		String s = stream.readString(maxLen);
		Serial.print(_F("tmpl: "));
		Serial.println(tmpl);
		Serial.print(_F(" res: "));
		Serial.println(s);
		Serial.print(_F(" ref: "));
		Serial.println(ref);
		REQUIRE(ref == s);
		return;
	}

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
