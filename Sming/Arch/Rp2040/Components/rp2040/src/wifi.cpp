/*
 * Contains code based on https://github.com/raspberrypi/pico-sdk/issues/909
 * NB. Future driver updates may simplify this process.
 */

#ifndef DISABLE_WIFI

#include <Storage.h>
#include <Storage/PartitionStream.h>
#include <Data/Stream/FlashMemoryStream.h>
#include <uzlib.h>
#include <debug_progmem.h>
extern "C" {
#include <cyw43_ll.h>
}

#ifdef CYW43_FIRMWARE
#include <FlashString/Array.hpp>
IMPORT_FSTR_ARRAY_LOCAL(cyw43_firmware, uint8_t, CYW43_FIRMWARE)
#endif

namespace
{
#define BUFFER_SIZE 16384
#define DICT_SIZE 32767

class Decompressor
{
public:
	explicit Decompressor(const FSTR::ObjectBase& data) : stream(new FlashMemoryStream(data))
	{
	}

	explicit Decompressor(Storage::Partition part) : stream(new Storage::PartitionStream(part))
	{
	}

	bool init()
	{
		uzlib_init();
		uzlib_uncompress_init(&state, dict, DICT_SIZE);
		state.source_read_cb = read_source;
		int res = uzlib_gzip_parse_header(&state);
		if(res != TINF_OK) {
			debug_e("[CYW] bad GZIP header %d", res);
			return false;
		}
		return true;
	}

	bool read(void* dest, size_t length)
	{
		state.dest = static_cast<uint8_t*>(dest);
		state.dest_limit = state.dest + length;
		int res = uzlib_uncompress_chksum(&state);
		if(res != TINF_OK) {
			debug_e("[CYW] Decompress error %d", res);
			return false;
		}
		return true;
	}

private:
	// Return -1 when finished
	static int read_source(struct uzlib_uncomp* uncomp)
	{
		auto self = reinterpret_cast<Decompressor*>(uncomp);
		size_t len = self->stream->readBytes(self->src_buffer, BUFFER_SIZE);
		if(len == 0) {
			return -1;
		}

		uncomp->source = self->src_buffer;
		uncomp->source_limit = &self->src_buffer[len];

		return *self->state.source++;
	}

	struct uzlib_uncomp state {
	};
	uint8_t src_buffer[BUFFER_SIZE]{};
	uint8_t dict[DICT_SIZE];
	std::unique_ptr<IDataSourceStream> stream;
};

std::unique_ptr<Decompressor> decompressor;

} // namespace

int cyw43_storage_init()
{
#ifdef CYW43_FIRMWARE
	decompressor = std::make_unique<Decompressor>(cyw43_firmware);
#else
	auto part = Storage::findPartition("cyw43_fw");
	if(!part) {
		debug_e("Failed to find CYW43 firmware partition");
	} else {
		decompressor = std::make_unique<Decompressor>(part);
	}
#endif

	if(!decompressor || !decompressor->init()) {
		decompressor.reset();
		return -1;
	}

	return 0;
}

uint32_t cyw43_storage_read(void* dest, uint32_t length)
{
	if(!decompressor) {
		return 0;
	}

	if(!decompressor->read(dest, length)) {
		decompressor.reset();
		return 0;
	}

	return length;
}

uint32_t cyw43_storage_get_chunksize()
{
	const uint32_t chunkTag = 0x4b4e4843; // "CHNK"
	struct chunk_t {
		uint32_t tag;
		uint32_t length;
	};
	struct chunk_t chunk;
	int res = cyw43_storage_read(&chunk, sizeof(chunk));
	if(res != sizeof(chunk)) {
		debug_e("[CYW43] Bad chunk header %d\n", res);
		return 0;
	}
	if(chunk.tag != chunkTag) {
		debug_e("[CYW43] Bad chunk tag %08x\n", chunk.tag);
		return 0;
	}
	debug_d("[CYW43] Chunk %u bytes\n", chunk.length);
	return chunk.length;
}

void cyw43_storage_cleanup()
{
	decompressor.reset();
}

#endif // ifndef DISABLE_WIFI
