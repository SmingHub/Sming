#include <HostTests.h>
#include <FlashString/TemplateStream.hpp>
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Stream/LimitedMemoryStream.h>
#include <Data/Stream/XorOutputStream.h>
#include <Data/Stream/SharedMemoryStream.h>
#include <Data/WebHelpers/base64.h>
#include <malloc_count.h>

#ifndef DISABLE_NETWORK
#include <Data/Stream/ChunkedStream.h>
#endif

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

		MallocCount::setLogThreshold(0);

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

		TEST_CASE("readString (PR #2468)")
		{
			FSTR::Stream stream(FS_abstract);
			String s = stream.readString(0x10000);
			REQUIRE(s == FS_abstract);
		}

#ifndef DISABLE_NETWORK

		TEST_CASE("ChunkedStream / StreamTransformer")
		{
			DEFINE_FSTR_LOCAL(FS_INPUT, "Some test data");
			DEFINE_FSTR_LOCAL(FS_OUTPUT, "E\r\nSome test data\r\n0\r\n\r\n");
			ChunkedStream chunked(new FlashMemoryStream(FS_INPUT));
			MemoryDataStream output;
			output.copyFrom(&chunked);
			String s;
			REQUIRE(output.moveString(s));
			m_printHex("OUTPUT", s.c_str(), s.length());
			REQUIRE(FS_OUTPUT == s);
		}

		TEST_CASE("MultipartStream / MultiStream")
		{
			unsigned itemIndex{0};
			constexpr const FlashString* items[]{
				&template1, &template1_1, &template1_2, &template2, &template2_1,
			};
			MultipartStream multi([&]() -> MultipartStream::BodyPart {
				MultipartStream::BodyPart part;
				if(itemIndex < ARRAY_SIZE(items)) {
					part.headers = new HttpHeaders;
					part.stream = new FSTR::Stream(*items[itemIndex++]);
				}
				return part;
			});

			// For testing, hack the boundary value so we can compare it against a reference output
			auto boundary = const_cast<char*>(multi.getBoundary());
			memcpy(boundary, "oALsXuO7vSbrvve", 16);

			MemoryDataStream mem;
			size_t copySize = mem.copyFrom(&multi);
			debug_i("copySize = %u", copySize);
			REQUIRE(int(copySize) == mem.available());
			String s;
			REQUIRE(mem.moveString(s));
			REQUIRE(Resource::multipart_result == s);
		}
#endif

		TEST_CASE("XorOutputStream")
		{
			auto mem = new MemoryDataStream();
			String input = "For testing, hack the boundary value so we can compare it against a reference output";
			mem->write(input.c_str(), input.length());

			uint8_t maskKey[4] = {0x00, 0x01, 0x02, 0x03};
			for(uint8_t x = 0; x < sizeof(maskKey); x++) {
				maskKey[x] = (char)os_random();
			}

			XorOutputStream encodeStream(mem, maskKey, sizeof(maskKey));
			auto maskedStream = new MemoryDataStream();
			while(!encodeStream.isFinished()) {
				char buffer[21];
				uint16_t obtained = encodeStream.readBytes(buffer, sizeof(buffer));
				maskedStream->write(buffer, obtained);
			}

			XorOutputStream decodeStream(maskedStream, maskKey, sizeof(maskKey));
			MemoryDataStream unmaskedStream;
			while(!decodeStream.isFinished()) {
				char buffer[13];
				uint16_t obtained = decodeStream.readBytes(buffer, sizeof(buffer));
				unmaskedStream.write(buffer, obtained);
			}

			String unmaskedString;
			unmaskedStream.moveString(unmaskedString);

			// running xor two times should produce the original content
			REQUIRE(input == unmaskedString);
			debug_hex(DBG, "Text", unmaskedString.c_str(), unmaskedString.length());
		}

		{
			// STL may perform one-time memory allocation for mutexes, etc.
			std::shared_ptr<const char[]> data(new char[18]);
			SharedMemoryStream stream(data, 18);
		}

		auto memStart = MallocCount::getCurrent();
		// auto memStart = system_get_free_heap_size();

		TEST_CASE("SharedMemoryStream")
		{
			const char* message = "Wonderful data...";
			const size_t msglen = strlen(message);
			std::shared_ptr<char[]> data(new char[msglen]);
			memcpy(data.get(), message, msglen);

			debug_d("RefCount: %d", data.use_count());

			Vector<SharedMemoryStream<const char[]>*> list;
			for(unsigned i = 0; i < 4; i++) {
				list.addElement(new SharedMemoryStream<const char[]>(data, msglen));
			}

			for(auto element : list) {
				constexpr size_t bufferSize{5};
				char buffer[bufferSize]{};

				String output;
				while(!element->isFinished()) {
					size_t consumed = element->readBytes(buffer, bufferSize);
					output.concat(buffer, consumed);
				}

				REQUIRE(output.equals(message));

				delete element;
				debug_d("RefCount: %d", data.use_count());
			}

			REQUIRE(data.use_count() == 1);
		}

		auto memNow = MallocCount::getCurrent();
		// auto memNow = system_get_free_heap_size();
		REQUIRE_EQ(memStart, memNow);
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
