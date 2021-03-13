/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BasicOtaUpgradeStream.h
 *
 ****/

#pragma once

#include <Data/Stream/ReadWriteStream.h>
#include <rboot-api.h>
#include "FileFormat.h"
#ifdef ENABLE_OTA_SIGNING
#include "SignatureVerifier.h"
#else
#include "ChecksumVerifier.h"
#endif

namespace OtaUpgrade
{
/**
 * @brief A write-only stream to parse and apply firmware unencrypted upgrade files generated by otatool.py
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
 * refer to the library's documentation.
 * 
 * @see `EncryptedStream` for encryption support.
 */
class BasicStream : public ReadWriteStream
{
public:
	BasicStream();

	/**
	 * @brief Error code values
	 */
	enum class Error {
		None,			  ///< No error occured thus far (default value of \c #errorCode if `hasError()` returns false)
		InvalidFormat,	///< Invalid/unsupported upgrade file format
		UnsupportedData,  ///< Some content of the upgrade file is not supported by this version of OtaUpgradeStream.
		DecryptionFailed, ///< Decryption failed. Probably wrong decryption key.
		NoRomFound,  ///< The file did not contain a ROM image suitable for the start address of the slot to upgrade.
		RomTooLarge, ///< The contained ROM image does not fit into the application firmware slot.
		DowngradeNotAllowed, ///< Attempt to downgrade to older firmware version.
		VerificationFailed,  ///< Signature/checksum verification failed - updated ROM not activated
		FlashWriteFailed,	///< Error while writing to Flash memory.
		RomActivationFailed, ///< Error while activating updated ROM slot.
		OutOfMemory,		 ///< Dynamic memory allocation failed
		Internal,			 ///< An unexpected error occured.
	};

	Error errorCode = Error::None; ///< Error code. Only relevant if `hasError()` returns `true`.

	/** @brief Convert error code to string.
	 * @see #errorCode
	 * @deprecated Use `toString()` global function
	 */
	static String errorToString(Error code) SMING_DEPRECATED;

	/** @brief Process chunk of upgrade file.
	 * @param data Pointer to chunk of data.
	 * @param size Size of chunk pointed to by \a data in bytes.
	 * @return If less than \a size, an error occured. Check \c #errorCode for more details.
	 * @note Even if `write()` never returns less than \a size it is not guaranteed that the upgrade was successful.
	 * 		 Always use `hasError()` to determine success.
	 */
	size_t write(const uint8_t* data, size_t size) override;

	/** @brief Returns true if an error happened during the upgrade process.
	 * @see `errorCode`
	 */
	bool hasError() const
	{
		return (state == State::Error);
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

protected:
	void setError(Error ec);

private:
	/** Representation of the ROM slot to receive the new firmware */
	struct Slot {
		/** Determine the parameters of the slot to receive the upgrade. */
		Slot();

		uint32_t address;
		uint32_t size;
		uint8_t index;
		bool updated{false};
	};
	Slot slot;

	// Instead of RbootOutputStream, the rboot write API is used directly because in a future extension the OTA file may contain data for multiple FLASH regions.
	rboot_write_status rbootWriteStatus{};

	enum class State {
		Error,
		Header,
		RomHeader,
		SkipRom,
		WriteRom,
		VerifyRoms,
		RomsComplete,
	};
	State state{State::Header};

#ifdef ENABLE_OTA_SIGNING
	using Verifier = SignatureVerifier;
	static const uint32_t expectedHeaderMagic{OTA_HEADER_MAGIC_SIGNED};
#else
	using Verifier = ChecksumVerifier;
	static const uint32_t expectedHeaderMagic{OTA_HEADER_MAGIC_NOT_SIGNED};
#endif
	Verifier verifier;

	OtaFileHeader fileHeader;
	OtaRomHeader romHeader;

	Verifier::VerificationData verificationData;

	size_t remainingBytes{0};
	uint8_t* destinationPtr{nullptr};
	uint8_t romIndex{0};

	void setupChunk(State nextState, size_t size, void* destination = nullptr)
	{
		state = nextState;
		remainingBytes = size;
		destinationPtr = reinterpret_cast<uint8_t*>(destination);
	}
	template <typename T> void setupChunk(State nextState, T& destination)
	{
		setupChunk(nextState, sizeof(T), &destination);
	}

	/** Helper function used by `write()`.
	 * It "consumes" as much bytes as possible for completing the current chunk (file header, signature, etc.) and
	 * updates \a data and \a size accordingly. Depending on `state`, this method also writes to flash memory and
	 * performs incremental checksum/signature calculation.
	 * @return `true` if the current chunk is complete and ready for processing. 
	 * 			It is the caller's responsibility to setup a new chunk, advance the state machine, etc.
	 */
	bool consume(const uint8_t*& data, size_t& size);

	/** Called after completion of a single ROM image. 
	 * Sets up reception of next ROM image or the checksum/signature if this was the laste ROM image.
	 */
	void nextRom();
	/** Called after reception of an #OTA_RomHeader. 
	 * Decides if the ROM fits the selected upgrade slot or must be ignored.
	 */
	void processRomHeader();
	/** Called after completion of all ROM images from the upgrade file to perform checksum/signature validation.
	 * If successful, the upgraded slot is set as active ROM using the rBoot API.
	 */
	void verifyRoms();
};

} // namespace OtaUpgrade

/** @brief Convert error code to string.
 * @see #errorCode
 */
String toString(OtaUpgrade::BasicStream::Error code);
