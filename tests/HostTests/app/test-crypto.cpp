#include <SmingTest.h>
#include <Crypto.h>
#include "crypto/AxHash.h"
#include "crypto/BrHash.h"

#ifdef ARCH_ESP8266
#include "crypto/EspHash.h"
#endif

IMPORT_FSTR_LOCAL(FS_plainText, PROJECT_DIR "/files/abstract.txt");
DEFINE_FSTR_LOCAL(FS_hmacKey, "very small key");

/*
 * State values as returned from BR SSL
 */
DEFINE_FSTR_LOCAL(MD5_STATE, "6faac17fdff31f873ad60de0b79477d6")
DEFINE_FSTR_LOCAL(SHA1_STATE, "2e209fc0505d0a8115ab157201f1b8fa9f857696")
DEFINE_FSTR_LOCAL(SHA224_STATE, "3f999b1bb140dbc02e019396d9e47678fc6508eabfa68f99861be595998e07cb")
DEFINE_FSTR_LOCAL(SHA256_STATE, "d53b6c79bd832292eb5c98c5f6fef5e1c48df21d2378c2b90e69b93f9855fd82")
DEFINE_FSTR_LOCAL(SHA384_STATE, "8e1bf5fea0e46f3a67de62b34e5279536a2d993d0096c494f57c56f83b2b2fdf89eab1390ff8e7e836854a"
								"a3c447c34c6a3aca08083d56fa4520a85d58e289d6")
DEFINE_FSTR_LOCAL(SHA512_STATE, "1668f13d9047fc988e5eb5f796d2820e4352f6deb88e81999738922dfa3979120451e5e46bab7303584992"
								"0a384a76985b56d9cd47016c8b45e80f0d4772db8d")

/*
 * https://www.fileformat.info/tool/hash.htm
 * https://coding.tools/sha224
 */
DEFINE_FSTR_LOCAL(MD5_HASH, "4c9f6bb2c9891159487912899f1f2db1")
DEFINE_FSTR_LOCAL(SHA1_HASH, "e4ba625521d569372fc25ca89acd7e9e5322ef27")
DEFINE_FSTR_LOCAL(SHA224_HASH, "b5d1bbb79b85097e8be475c02811a588a3da8221fb72dc45d2a17afe")
DEFINE_FSTR_LOCAL(SHA256_HASH, "b23a4fd1eccb0a4779665c85cb05802cac583e51b7be126cb61a73660dd1059a")
DEFINE_FSTR_LOCAL(SHA384_HASH,
				  "85c84af8a4071fe37d5a23e1b5aae421203f633c684a081e8c5e9850848cf1d52f034b49cc4eef241631feb9a5d78d6b")
DEFINE_FSTR_LOCAL(SHA512_HASH, "a57b701e185ce83b0acb6985bd865e720cf945f66df2e534eca342247d09c5a3f9c1a100bdf2166c560a3a2"
							   "e22ecb822bcd8b91abda6dc7f609dad6a863e50c9")

/*
 * https://hash.online-convert.com/md5-generator
 */
DEFINE_FSTR_LOCAL(MD5_HMAC, "da512014e8f20e3c139cce4d1770a36b")
DEFINE_FSTR_LOCAL(SHA1_HMAC, "d191a9454a2345c4c4d4a07ad84d16b52815ae42")
DEFINE_FSTR_LOCAL(SHA224_HMAC, "b885dccf67014b88b6196e1156f1d988ecdf13c49e8dec60c7df5dcd")
DEFINE_FSTR_LOCAL(SHA256_HMAC, "68ee9b090f274c4eabbe4f96721ca3bb5f054d9690129f83734af64bb33e36a3")
DEFINE_FSTR_LOCAL(SHA384_HMAC,
				  "8221e55eb5261aba1367830b8a9327177e5770b56585e49ba25cd195f129157621023d0d83f1d58b1bef11e7bcf2edc0")
DEFINE_FSTR_LOCAL(SHA512_HMAC, "b5ca49d3db33f14ec7eb8f76795d8744463a8473c24b9f1a7e48d381d291f45cd131c690376952342801eda"
							   "a327b132230479d304760e8bfe96fa615e623793a")

class CryptoTest : public TestGroup
{
public:
	static constexpr unsigned iterations = 100;

	CryptoTest() : TestGroup(_F("crypto")), hmacKey(FS_hmacKey), plainText(FS_plainText)
	{
	}

	void execute() override
	{
		nextTest();
	}

	template <class Context> void checkHash(const FlashString& expectedHash, FlashString expectedState = {})
	{
		Context ctx;
		ctx.update(FS_plainText);
		auto state = ctx.getState();
		auto hash = ctx.getHash();

		auto stateText = Crypto::toString(state.value);
		auto hashText = Crypto::toString(hash);

		Serial.println(Context::Engine::name);
		if(!expectedState.isNull()) {
			Serial.print(_F("  state: "));
			Serial.println(stateText);
			Serial.print(_F("  count: "));
			Serial.println(state.count);
		}
		Serial.print(_F("  final: "));
		Serial.println(hashText);
		Serial.print(_F("  context size: "));
		Serial.println(sizeof(Context));
		if(!expectedState.isNull()) {
			REQUIRE(state.count == FS_plainText.length());
			REQUIRE(Crypto::toString(state.value) == expectedState);
		}
		REQUIRE(hashText == expectedHash);
	}

	template <class Context> void checkHmac(const FlashString& expectedHash)
	{
		auto hash = Context::calculate(FS_hmacKey, FS_plainText);
		auto hashText = Crypto::toString(hash);
		Serial.print(Context::Engine::name);
		Serial.print(": ");
		Serial.println(hashText);
		REQUIRE(hashText == expectedHash);
	}

	template <class Context> void benchmarkHash(const String& expected)
	{
		MicroTimes times(Context::Engine::name);
		for(unsigned i = 0; i < iterations; ++i) {
			times.start();
			auto hash = Context::calculate(plainText);
			times.update();
			assert(Crypto::toString(hash) == expected);
		}
		Serial.println(times);
	}

	template <class Context> void benchmarkHmac(const String& expected)
	{
		MicroTimes times(Context::Engine::name);
		for(unsigned i = 0; i < iterations; ++i) {
			times.start();
			auto hash = Context::calculate(hmacKey, plainText);
			times.update();
			assert(Crypto::toString(hash) == expected);
		}
		Serial.println(times);
	}

	void benchmarkFunction(const String& title, Delegate<void()> func)
	{
		MicroTimes times(title);
		for(unsigned i = 0; i < iterations; ++i) {
			times.start();
			func();
			times.update();
		}
		Serial.println(times);
	}

	void nextTest()
	{
		switch(state) {
		case 0:
			TEST_CASE("Crypto Hashes")
			{
				checkHash<Crypto::Md5>(MD5_HASH, MD5_STATE);
				checkHash<Crypto::Sha1>(SHA1_HASH, SHA1_STATE);
				checkHash<Crypto::Sha224>(SHA224_HASH, SHA224_STATE);
				checkHash<Crypto::Sha256>(SHA256_HASH, SHA256_STATE);
				checkHash<Crypto::Sha384>(SHA384_HASH, SHA384_STATE);
				checkHash<Crypto::Sha512>(SHA512_HASH, SHA512_STATE);
			}
			break;

		case 1:
			TEST_CASE("axTLS Hashes")
			{
				checkHash<Crypto::Ax::Md5>(MD5_HASH);
				checkHash<Crypto::Ax::Sha1>(SHA1_HASH);
				checkHash<Crypto::Ax::Sha256>(SHA256_HASH);
				checkHash<Crypto::Ax::Sha384>(SHA384_HASH);
				checkHash<Crypto::Ax::Sha512>(SHA512_HASH);
			}
			break;

		case 2:
			TEST_CASE("BearSSL Hashes")
			{
				checkHash<Crypto::Br::Md5>(MD5_HASH, MD5_STATE);
				checkHash<Crypto::Br::Sha1>(SHA1_HASH, SHA1_STATE);
				checkHash<Crypto::Br::Sha224>(SHA224_HASH, SHA224_STATE);
				checkHash<Crypto::Br::Sha256>(SHA256_HASH, SHA256_STATE);
				checkHash<Crypto::Br::Sha384>(SHA384_HASH, SHA384_STATE);
				checkHash<Crypto::Br::Sha512>(SHA512_HASH, SHA512_STATE);
			}
			break;

		case 3:
			TEST_CASE("HMAC")
			{
				checkHmac<Crypto::HmacMd5>(MD5_HMAC);
				checkHmac<Crypto::HmacSha1>(SHA1_HMAC);
				checkHmac<Crypto::HmacSha224>(SHA224_HMAC);
				checkHmac<Crypto::HmacSha256>(SHA256_HMAC);
				checkHmac<Crypto::HmacSha384>(SHA384_HMAC);
				checkHmac<Crypto::HmacSha512>(SHA512_HMAC);
			}
			break;

		case 4:
			TEST_CASE("Benchmark Crypto Hashes")
			{
				benchmarkHash<Crypto::Md5>(MD5_HASH);
				benchmarkHash<Crypto::Sha1>(SHA1_HASH);
				benchmarkHash<Crypto::Sha224>(SHA224_HASH);
				benchmarkHash<Crypto::Sha256>(SHA256_HASH);
				benchmarkHash<Crypto::Sha384>(SHA384_HASH);
				benchmarkHash<Crypto::Sha512>(SHA512_HASH);
			}
			break;

		case 5:
#ifdef ARCH_ESP8266
			TEST_CASE("Benchmark ESP Hashes")
			{
				benchmarkHash<Crypto::Esp::Md5>(MD5_HASH);
				benchmarkHash<Crypto::Esp::Sha1>(SHA1_HASH);
			}
#endif
			break;

		case 6:
			TEST_CASE("Benchmark axTLS Hashes")
			{
				benchmarkHash<Crypto::Ax::Md5>(MD5_HASH);
				benchmarkHash<Crypto::Ax::Sha1>(SHA1_HASH);
				benchmarkHash<Crypto::Ax::Sha256>(SHA256_HASH);
				benchmarkHash<Crypto::Ax::Sha384>(SHA384_HASH);
				benchmarkHash<Crypto::Ax::Sha512>(SHA512_HASH);
			}
			break;

		case 7:
			TEST_CASE("Benchmark BearSSL Hashes")
			{
				benchmarkHash<Crypto::Br::Md5>(MD5_HASH);
				benchmarkHash<Crypto::Br::Sha1>(SHA1_HASH);
				benchmarkHash<Crypto::Br::Sha224>(SHA224_HASH);
				benchmarkHash<Crypto::Br::Sha256>(SHA256_HASH);
				benchmarkHash<Crypto::Br::Sha384>(SHA384_HASH);
				benchmarkHash<Crypto::Br::Sha512>(SHA512_HASH);
			}
			break;

		case 8:
			TEST_CASE("Benchmark HMAC")
			{
				benchmarkHmac<Crypto::HmacMd5>(MD5_HMAC);
				benchmarkHmac<Crypto::HmacSha1>(SHA1_HMAC);
				benchmarkHmac<Crypto::HmacSha224>(SHA224_HMAC);
				benchmarkHmac<Crypto::HmacSha256>(SHA256_HMAC);
				benchmarkHmac<Crypto::HmacSha384>(SHA384_HMAC);
				benchmarkHmac<Crypto::HmacSha512>(SHA512_HMAC);
			}
			break;

		case 9:
			TEST_CASE("Benchmark HMAC functions")
			{
				Crypto::Blob key(hmacKey);
				Crypto::Blob text(plainText);
#ifdef ARCH_ESP8266
				benchmarkFunction(_F("ESP_hmac_md5"), [&]() {
					Crypto::Md5::Hash hash;
					ESP_hmac_md5(key.data(), key.size(), text.data(), text.size(), hash.data());
				});
				benchmarkFunction(_F("ESP_hmac_sha1"), [&]() {
					Crypto::Sha1::Hash hash;
					ESP_hmac_sha1(key.data(), key.size(), text.data(), text.size(), hash.data());
				});
#endif
				benchmarkFunction(_F("ax_hmac_md5"), [&]() {
					Crypto::Md5::Hash hash;
					ax_hmac_md5(text.data(), text.size(), key.data(), key.size(), hash.data());
				});
				benchmarkFunction(_F("ax_hmac_sha1"), [&]() {
					Crypto::Sha1::Hash hash;
					ax_hmac_sha1(text.data(), text.size(), key.data(), key.size(), hash.data());
				});
				benchmarkFunction(_F("ax_hmac_sha256"), [&]() {
					Crypto::Sha256::Hash hash;
					ax_hmac_sha256(text.data(), text.size(), key.data(), key.size(), hash.data());
				});
			}
			break;

		default:
			complete();
			return;
		}

		++state;
		System.queueCallback([this]() { nextTest(); });
		pending();
	}

private:
	unsigned state = 0;
	// Pre-load this from flash so as not to skew benchmarks
	String hmacKey;
	String plainText;
};

void REGISTER_TEST(crypto)
{
	registerGroup<CryptoTest>();
}
