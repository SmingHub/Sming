#include "SignedRbootOutputStream.h"

SignedRbootOutputStream::SignedRbootOutputStream(int32_t startAddress, size_t maxLength, const uint8_t* verificationKey)
	: RbootOutputStream(startAddress, maxLength), verificationKey(verificationKey),
	  headerPtr(reinterpret_cast<uint8_t*>(&header)), missingHeaderBytes(sizeof(header)), okFlag(true),
	  startAddress(startAddress)
{
	crypto_sign_init(&verifierState);
}
size_t SignedRbootOutputStream::write(const uint8_t* data, size_t size)
{
	size_t consumed = 0;
	if(okFlag) {
		if(missingHeaderBytes > 0) {
			const size_t chunkSize = std::min(size, missingHeaderBytes);
			memcpy(headerPtr, data, chunkSize);
			headerPtr += chunkSize;
			missingHeaderBytes -= chunkSize;
			data += chunkSize;
			size -= chunkSize;
			consumed += chunkSize;
			if(missingHeaderBytes == 0) {
				debug_i("Receive image for load address 0x%08X, slot starts at 0x%08X\n", header.loadAddress,
						startAddress);
				const bool magicOk = (header.magic == HEADER_MAGIC_EXPECTED);
				const bool loadAddressOk = ((header.loadAddress & 0x000FFFFF) == (startAddress & 0x000FFFFF));
				okFlag = magicOk && loadAddressOk;
				if(!okFlag) {
					setError(magicOk ? "Unexpected load address. Try image for other slot."
									 : "Invalid image received.");
					return 0;
				}

				init();
			}
		}

		if(size > 0) {
			crypto_sign_update(&verifierState, static_cast<const unsigned char*>(data), size);
			consumed += RbootOutputStream::write(data, size);
		}
	}
	return consumed;
}

bool SignedRbootOutputStream::close()
{
	if(closeCalled) {
		return true;
	}
	closeCalled = true;

	if(okFlag) {
		if(!RbootOutputStream::close()) {
			return false;
		}

		const bool signatureMatch = (crypto_sign_final_verify(&verifierState, header.signature, verificationKey) == 0);
		if(!signatureMatch) {
			// destroy start sector of updated ROM to avoid accidental booting an unsanctioned firmware
			spi_flash_erase_sector(startAddress / SECTOR_SIZE);
			setError("Signature mismatch");
		}
	}
	return okFlag;
}
