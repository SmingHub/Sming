#include <HostTests.h>

#include <Data/Stream/MemoryDataStream.h>
#include <Data/Stream/Base64OutputStream.h>
#include <Data/WebHelpers/base64.h>

class Base64Test : public TestGroup
{
public:
	Base64Test() : TestGroup(_F("Base64"))
	{
	}

	void execute() override
	{
		libTests();
		streamTests();
	}

	void libTests()
	{
		String user("donkey");
		String pass("kingpin");
		String token = '\0' + user + '\0' + pass;
		String hash;

		TEST_CASE("Encode")
		{
			debug_hex(INFO, "encode input", token.c_str(), token.length() + 1);
			hash = base64_encode(token);
			debug_hex(INFO, "encode output", hash.c_str(), hash.length() + 1);
			REQUIRE(hash == _F("AGRvbmtleQBraW5ncGlu"));
		}

		TEST_CASE("Decode")
		{
			String clear = base64_decode(hash);
			debug_hex(INFO, "decode output", clear.c_str(), clear.length() + 1);
			REQUIRE(clear == token);
		}

		TEST_CASE("Encode lengths")
		{
			// Verify that actual encoded size is no larger than estimated size
			constexpr size_t inputSize{1024};
			constexpr size_t outputSize{2048};
			auto inbuf = new uint8_t[inputSize];
			auto outbuf = new char[outputSize];
			os_get_random(inbuf, inputSize);
			for(unsigned i = 0; i < inputSize; ++i) {
				auto minEncodeLen = base64_min_encode_len(i);
				int len = base64_encode(i, inbuf, outputSize, outbuf);
				CHECK(len >= int(i));
				CHECK(size_t(len) <= minEncodeLen);
			}
			delete[] outbuf;
			delete[] inbuf;
		}

		TEST_CASE("Decode lengths")
		{
			// Verify that actual decoded size is no larger than estimated size
			constexpr size_t inputSize{2048};
			constexpr size_t outputSize{1020};
			auto inbuf = new char[inputSize];
			auto outbuf = new uint8_t[outputSize];
			os_get_random(reinterpret_cast<uint8_t*>(outbuf), outputSize);
			size_t maxLen = base64_encode(outputSize, outbuf, inputSize, inbuf);
			CHECK(maxLen < inputSize);
			for(unsigned i = 0; i < maxLen; ++i) {
				auto minDecodeLen = base64_min_decode_len(i);
				int len = base64_decode(i, inbuf, outputSize, outbuf);
				CHECK(len <= int(i));
				CHECK(size_t(len) <= minDecodeLen);
			}
			delete[] outbuf;
			delete[] inbuf;
		}
	}

	void streamTests()
	{
		TEST_CASE("Base64OutputStream / StreamTransformer")
		{
			auto src = new FSTR::Stream(Resource::image_png);
			Base64OutputStream base64stream(src);
			MemoryDataStream output;
			output.copyFrom(&base64stream);
			String s;
			REQUIRE(output.moveString(s));
			s = base64_decode(s);
			REQUIRE(Resource::image_png == s);
		}
	}
};

void REGISTER_TEST(Base64)
{
	registerGroup<Base64Test>();
}
