#include <SmingTest.h>
#include <Crypto.h>

IMPORT_FSTR_LOCAL(plainText, PROJECT_DIR "/files/abstract.txt");
DEFINE_FSTR_LOCAL(hmacKey, "very small key");

class CryptoTest : public TestGroup
{
public:
	CryptoTest() : TestGroup(_F("crypto"))
	{
	}

	template <class Hash> void check(const Hash& hash, const String& expected)
	{
		auto s = toString(hash);
		Serial.print(Hash::Engine::name);
		Serial.print(" = ");
		Serial.println(s);
		REQUIRE(s == expected);
	}

	/*
	 * Verification values obtained via:
	 *
	 * 		https://www.fileformat.info/tool/hash.htm
	 * 		https://hash.online-convert.com/md5-generator
	 */
	void execute() override
	{
		TEST_CASE("Hashes")
		{
#define CTX_CHECK(Context, expected) check(Context::calculate(plainText), F(expected))
			CTX_CHECK(Crypto::Md5, "4c9f6bb2c9891159487912899f1f2db1");
			CTX_CHECK(Crypto::Sha1, "e4ba625521d569372fc25ca89acd7e9e5322ef27");
			CTX_CHECK(Crypto::Sha256, "b23a4fd1eccb0a4779665c85cb05802cac583e51b7be126cb61a73660dd1059a");
		}

		TEST_CASE("HMAC")
		{
#define HMAC_CHECK(Hmac, expected) check(Hmac::calculate(String(plainText), String(hmacKey)), expected)
			HMAC_CHECK(DECL((Crypto::Hmac<Crypto::Md5, 64>)), F("da512014e8f20e3c139cce4d1770a36b"));
			HMAC_CHECK(DECL((Crypto::Hmac<Crypto::Sha1, 64>)), F("d191a9454a2345c4c4d4a07ad84d16b52815ae42"));
			HMAC_CHECK(DECL((Crypto::Hmac<Crypto::Sha256, 64>)),
					   F("68ee9b090f274c4eabbe4f96721ca3bb5f054d9690129f83734af64bb33e36a3"));
		}
	}
};

void REGISTER_TEST(crypto)
{
	registerGroup<CryptoTest>();
}
