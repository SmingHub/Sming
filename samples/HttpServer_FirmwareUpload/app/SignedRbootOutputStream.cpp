#include "SignedRbootOutputStream.h"
#include <esp_spi_flash.h>

SignedRbootOutputStream::SignedRbootOutputStream(int32_t startAddress, size_t maxLength)
	: RbootOutputStream(startAddress, maxLength), headerPtr(reinterpret_cast<uint8_t*>(&header)),
	  missingHeaderBytes(sizeof(header)), errorFlag(false), startAddress(startAddress)
{
	crypto_sign_init(&verifierState);
}

void SignedRbootOutputStream::setError(const char* message)
{
	errorFlag = true;
	errorMessage = message;
	debug_e("%s", errorMessage.c_str());
}

size_t SignedRbootOutputStream::write(const uint8_t* data, size_t size)
{
	size_t consumed = 0;
	if(!errorFlag) {
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
				const bool unexpectedMagic = (header.magic != HEADER_MAGIC_EXPECTED);
				const bool unexpectedLoadAddress = ((header.loadAddress & 0x000FFFFF) != (startAddress & 0x000FFFFF));
				if(unexpectedMagic || unexpectedLoadAddress) {
					setError(unexpectedLoadAddress ? "Unexpected load address. Try image for other slot."
												   : "Invalid image received.");
					return 0;
				}

				init();
			}
		}

		if(size > 0) {
			crypto_sign_update(&verifierState, static_cast<const unsigned char*>(data), size);
			const size_t written = RbootOutputStream::write(data, size);
			consumed += written;
			if(written != size) {
				setError("Flash write failure");
			}
		}
	}
	return consumed;
}

bool SignedRbootOutputStream::verifySignature(const uint8_t* verificationKey)
{
	if(!errorFlag) {
		// write final batch of of data
		if(RbootOutputStream::close()) {
			const bool signatureMatch =
				(crypto_sign_final_verify(&verifierState, header.signature, verificationKey) == 0);
			if(!signatureMatch) {
				// destroy start sector of updated ROM to avoid accidental booting an unsanctioned firmware
				flashmem_erase_sector(startAddress / SECTOR_SIZE);
				setError("Signature mismatch");
			}
		} else {
			setError("Flash write failure");
		}
	}
	return !errorFlag;
}
