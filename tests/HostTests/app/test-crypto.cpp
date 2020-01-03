#include <SmingTest.h>
#include <Crypto.h>

class CryptoTest : public TestGroup
{
public:
	CryptoTest() : TestGroup(_F("crypto"))
	{
	}

	void execute() override
	{
		String s = F("haggis basher");
		Serial.print("MD5 = ");
		Serial.println(Crypto::Md5::calculate(s).toString());
		Serial.print("Sha1 = ");
		Serial.println(Crypto::Sha1::calculate(s).toString());
		Serial.print("Sha256 = ");
		Serial.println(Crypto::Sha256::calculate(s).toString());
		Serial.print("HMAC.MD5 = ");
		Serial.println(Crypto::Hmac<Crypto::Md5, 64>::calculate(s, F("very small key")).toString());
	}
};

void REGISTER_TEST(crypto)
{
	registerGroup<CryptoTest>();
}
