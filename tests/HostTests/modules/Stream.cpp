#include <HostTests.h>
#include <FlashString/TemplateStream.hpp>
#include <Data/Stream/MemoryDataStream.h>

DEFINE_FSTR_LOCAL(template1, "Stream containing {var1}, {var2} and {var3}. {} {{}} {{12345")
DEFINE_FSTR_LOCAL(template1_1, "Stream containing value #1, value #2 and {var3}. {} {{}} {{12345")
DEFINE_FSTR_LOCAL(template1_2, "Stream containing value #1, value #2 and [value #3]. {} {{}} {{12345")

DEFINE_FSTR_LOCAL(template2, "This text should {disable}not {var1} really {var2:hello} again {enable}be missing.")
DEFINE_FSTR_LOCAL(template2_1, "This text should be missing.")

class StreamTest : public TestGroup
{
public:
	StreamTest() : TestGroup(_F("Stream"))
	{
	}

	void execute() override
	{
		const FlashString& FS_abstract = Resource::abstract_txt;

		TEST_CASE("MemoryDataStream::moveString")
		{
			FSTR::Stream src(FS_abstract);
			MemoryDataStream dest;
			dest.copyFrom(&src);
			String s;
			REQUIRE(dest.moveString(s) == true);
			REQUIRE(FS_abstract == s);
		}

		TEST_CASE("MemoryDataStream(String&&)")
		{
			// Move Stream into String
			String s(FS_abstract);
			MemoryDataStream stream(std::move(s));
			TEST_ASSERT(!s);
			TEST_ASSERT(FS_abstract.length() == size_t(stream.available()));
			// And back again
			stream.moveString(s);
			TEST_ASSERT(FS_abstract == s);
			REQUIRE(strlen(s.c_str()) == s.length());
		}

		TEST_CASE("LimitedMemoryStream::moveString (1)")
		{
			FSTR::Stream src(FS_abstract);
			REQUIRE(size_t(src.available()) == FS_abstract.length());
			LimitedMemoryStream dest(FS_abstract.length());
			// Operation will drop last character as this is converted to NUL
			dest.copyFrom(&src);
			String s;
			REQUIRE(dest.moveString(s) == false);
			debug_i("src length = %u, s length = %u", FS_abstract.length(), s.length());
			String s1(FS_abstract);
			s1.remove(s1.length() - 1);
			REQUIRE(s1 == s);
			REQUIRE(strlen(s.c_str()) == s.length());
		}

		TEST_CASE("LimitedMemoryStream::moveString (2)")
		{
			FSTR::Stream src(FS_abstract);
			REQUIRE(size_t(src.available()) == FS_abstract.length());
			LimitedMemoryStream dest(FS_abstract.length() + 10);
			// Operation will drop last character as this is converted to NUL
			dest.copyFrom(&src);
			String s;
			REQUIRE(dest.moveString(s) == true);
			REQUIRE(FS_abstract == s);
			REQUIRE(strlen(s.c_str()) == s.length());
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

void REGISTER_TEST(Stream)
{
	registerGroup<StreamTest>();
}
