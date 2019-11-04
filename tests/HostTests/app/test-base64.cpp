#include "common.h"

#include <Network/WebHelpers/base64.h>

class Base64Test : public TestGroup
{
public:
	Base64Test() : TestGroup(_F("Base64"))
	{
	}

	void execute() override
	{
		startTest("Encode");
		String user("donkey");
		String pass("kingpin");
		String token = '\0' + user + '\0' + pass;
		debug_hex(INFO, "encode input", token.c_str(), token.length() + 1);
		String hash = base64_encode(token);
		debug_hex(INFO, "encode output", hash.c_str(), hash.length() + 1);
		REQUIRE(hash == _F("AGRvbmtleQBraW5ncGlu"));

		String clear = base64_decode(hash);
		debug_hex(INFO, "decode output", clear.c_str(), clear.length() + 1);
		REQUIRE(clear == token);
	}
};

void REGISTER_TEST(base64)
{
	registerGroup<Base64Test>();
}
