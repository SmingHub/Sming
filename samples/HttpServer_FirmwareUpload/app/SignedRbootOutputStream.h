#include <Data/Stream/RbootOutputStream.h>
#include <sodium/crypto_sign.h>

class SignedRbootOutputStream : public RbootOutputStream
{
	struct {
		uint32_t magic;
		uint32_t loadAddress;
		uint8_t signature[crypto_sign_BYTES];
	} header;

	uint8_t* headerPtr;
	size_t missingHeaderBytes;
	static const uint32_t HEADER_MAGIC_EXPECTED = 0xf01af02a;

	bool errorFlag;
	uint32_t startAddress;

	crypto_sign_state verifierState;

	void setError(const char* message);

public:
	SignedRbootOutputStream(int32_t startAddress, size_t maxLength);

	String errorMessage;

	bool hasError() const
	{
		return errorFlag;
	}

	size_t write(const uint8_t* data, size_t size) override;

	bool verifySignature(const uint8_t* verificationKey);
};
