#ifndef OTA_UPGRADE_STREAM_H
#define OTA_UPGRADE_STREAM_H

#include <Data/Stream/ReadWriteStream.h>
#include <rboot-api.h>
#include <sodium/crypto_sign.h>

/* OTA File format
 * 4 Byte Magic: TBD
 * 2x4 Byte build timestamp (milliseconds since 1900) 
 * 1 Byte: number of roms to follow (currently 1 or 2, may be 0 in the future)
 * 3 Bytes: reserved for future use (set to zero)
 * n ROMs consisting of:
 * 	- 4 Byte flash offset
 * 	- 4 Byte Length
 * 	- ROM Content (Length bytes)
 * 16 Byte cryptographic signature (over everything since beginning of file)
 * Optional extensions may follow in the future for
 * - updating rboot
 * - updating RF parameter blob
 * - update/clear system parameters area
 * - Files or file system updates
 */
class OtaUpgradeStream : public ReadWriteStream
{
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

	static const uint32_t HEADER_MAGIC_EXPECTED = 0xf01af02a;

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
			
		uint8_t signature[crypto_sign_BYTES];
	};

	size_t remainingBytes = 0;
	uint8_t *destinationPtr = nullptr;
	uint8_t romIndex = 0;

	crypto_sign_state verifierState;

	void setError(const char *message = nullptr); 

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

public:
	OtaUpgradeStream();

	size_t write(const uint8_t* data, size_t size) override;

	bool hasError() const
	{
		return (state == StateError);
	}

	String errorMessage;
	
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
};

#endif // OTA_UPGRADE_STREAM_H
