/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * OtaUpgradeStream.h
 *
 ****/

#pragma once

#include <Data/Stream/ReadWriteStream.h>
#include <rboot-api.h>
#ifdef OTA_SIGNED
#include <sodium/crypto_sign.h>
#endif
#ifdef OTA_ENCRYPTED
#include <sodium/crypto_secretstream_xchacha20poly1305.h>
#endif

/**
 * @brief Parse and apply firmware upgrade files generated by otatool.py
 * 
 * A Write-only stream that parses and processed firmware upgrade files generated
 * by otatool.py.
 *
 * The class fully automates the firmware upgrade process without any manual 
 * configuration. At construction time, the rBoot configuration is read to determine 
 * the unused ROM slot which should receive the upgrade.
 * Just feed the upgrade file content into the `write()` method in arbitrarily 
 * sized chunks. The relevant portion(s) of the Flash memory (currently only the 
 * application rom) are updated on the fly as data arrives. When the file is complete
 * and signature validation (if enabled) was successful, the updated slot is activated
 * in the rBoot configuration.
 * Call `hasError()` and/or check the public \c #errorCode member to determine if 
 * everything went smoothly.
 * 
 * For further information on configuration options and the file format, 
 * refer to the component's documentation.

 * @note: If encryption is enabled, the class dynamically allocates a buffer to fit
 * the largest chunk of the encryption container (2kB unless otatool.py was modified).
 * Otherwise, no dynamic memory allocation is performed.
 */
class OtaUpgradeStream : public ReadWriteStream
{
public:
	OtaUpgradeStream();
	~OtaUpgradeStream();

	enum ErrorCode {
		NoError, ///< No error occured thus far (default value of \c #errorCode if `hasError()` returns false)
		InvalidFormatError, ///< Invalid/unsupported upgrade file format
		UnsupportedDataError, ///< Some content of the upgrade file is not supported by this version of OtaUpgradeStream.
		DecryptionError, ///< Decryption failed. Probably wrong decryption key.
		NoRomFoundError, ///< The file did not contain a ROM image suitable for the start address of the slot to upgrade.
		RomTooLargeError, ///< The contained ROM image does not fit into the application firmware slot.
		DowngradeError, ///< Attempt to downgrade to older firmware version.
		VerificationError, ///< Signature verification failed - updated ROM not activated
		FlashWriteError, ///< Error while writing to Flash memory.
		RomActivationError, ///< Error while activating updated ROM slot.
		OutOfMemoryError, ///< Dynamic memory allocation failed
		InternalError ///< An unexpected error occured.
	};

	ErrorCode errorCode = NoError; ///< Error code. Only relevant if `hasError()` returns true.

	/** @brief Convert error code to string.
	 * @see `errorCode`
	 */
	static String errorToString(ErrorCode code);

	/** @brief Process chunk of upgrade file
	 * @param data Pointer to chunk of data.
	 * @param size Size of chunk pointed to by `data` in bytes.
	 * @return If less than `size`, an error occured. Check \c #errorCode for more details.
	 * @note Even if `write()` never returns less than `size` it is not guaranteed that the upgrade was successful.
	 * 		 Always use `hasError()` to determine success.
	 */
	size_t write(const uint8_t* data, size_t size) override;

	/** @brief Returns true if an error happened during the upgrade process.
	 * @see `errorCode`
	 */
	bool hasError() const
	{
		return (state == StateError);
	}
	
	// overrides from IDataSourceStream
	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		return 0;
	}
	virtual int available() override
	{
		return 0;
	}
	bool isFinished() override
	{
		return true;
	}

private:
	struct Slot
	{
		uint8_t index;
		uint32_t address;
		uint32_t size;
		bool updated = false;

		Slot();
	} slot;

	// Instead of RbootOutputStream, the rboot write API is used directly because in a future extension the OTA file may contain data for multiple FLASH regions.
	rboot_write_status rbootWriteStatus = {};

	enum State {
		StateError,
		StateHeader,
		StateRomHeader,
		StateSkipRom,
		StateWriteRom,
		StateRomSignature,
		StateRomsComplete
	} state = StateHeader;

	struct {
		uint32_t magic;
		uint32_t buildTimestampLow;
		uint32_t buildTimestampHigh;
		struct {
			uint8_t romCount;
			uint8_t reserved[3];
		};
	} fileHeader;

	union {
		struct {
			uint32_t address;
			uint32_t size;
		} romHeader;
#ifdef OTA_SIGNED
		uint8_t signature[crypto_sign_BYTES];
#else
		uint8_t signature[16]; // md5sum
#endif
	};

	size_t remainingBytes = 0;
	uint8_t *destinationPtr = nullptr;
	uint8_t romIndex = 0;

#ifdef OTA_SIGNED
	crypto_sign_state verifierState;
#else
	struct {
		// see esptool/flasher_stub/rom_functions.h
		uint32_t state[4];
		uint32_t count[2];
		uint8_t buffer[64];
	} md5Context;
#endif

#ifdef OTA_ENCRYPTED
	struct {
		crypto_secretstream_xchacha20poly1305_state state;

		union {
			uint8_t header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
			uint16_t chunkSizeMinusOne;
		};

		enum {
			FragmentHeader,
			FragmentChunkSize,
			FragmentChunk,
			FragmentNone
		} fragment = FragmentHeader;
		
		size_t remainingBytes;
		uint8_t *fragmentPtr;
		uint8_t *buffer = nullptr;
		size_t bufferSize = 0;
	} encryption;
#endif

	void setError(ErrorCode ec);

	size_t process(const uint8_t* data, size_t size);

	void setupChunk(State nextState, size_t size, void *destination = nullptr)
	{
		state = nextState;
		remainingBytes = size;
		destinationPtr = reinterpret_cast<uint8_t *>(destination);
	}
	template <typename T>
	void setupChunk(State nextState, T& destination) {
		setupChunk(nextState, sizeof(T), &destination);
	}

	bool consume(const uint8_t *&data, size_t &size);

	void nextRom();
	void processRomHeader();
	void verifyRoms();
};