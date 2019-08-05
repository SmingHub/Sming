#include <Data/Stream/RbootOutputStream.h>
#include <sodium/crypto_sign.h>

class SignedRbootOutputStream : public RbootOutputStream
{
	const uint8_t* const verificationKey;

	struct {
		uint32_t magic;
		uint32_t loadAddress;
		uint8_t signature[crypto_sign_BYTES];
	} header;

	uint8_t* headerPtr;
	size_t missingHeaderBytes;
	static const uint32_t HEADER_MAGIC_EXPECTED = 0xf01af02a;

	bool okFlag;
	bool closeCalled = false;
	uint32_t startAddress;

	crypto_sign_state verifierState;

	void setError(const char* message)
	{
		okFlag = false;
		errorMessage = message;
		debug_e(errorMessage);
	}

public:
	SignedRbootOutputStream(int32_t startAddress, size_t maxLength, const uint8_t* verificationKey);

	String errorMessage;

	bool ok() const
	{
		return okFlag;
	}

	size_t write(const uint8_t* data, size_t size) override;

	bool close() override;
};
