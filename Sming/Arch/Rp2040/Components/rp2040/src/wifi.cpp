#ifndef DISABLE_WIFI

#include <Storage.h>
#include <debug_progmem.h>
extern "C" {
#include <cyw43_ll.h>
}

#ifdef CYW43_FIRMWARE
#include <FlashString/Array.hpp>
IMPORT_FSTR_ARRAY_LOCAL(cyw43_firmware, uint8_t, CYW43_FIRMWARE)
#endif

extern "C" uint32_t cyw43_storage_read(uint8_t *dest, uint32_t offset, uint32_t length)
{
#ifdef CYW43_FIRMWARE
	return cyw43_firmware.read(offset, dest, length);
#else
	static Storage::Partition part;
	if(!part) {
		part = Storage::findPartition("cyw43_fw");
		if(!part) {
			debug_e("Failed to find CYW43 firmware partition");
			return 0;
		}
	}

	if(offset + length > part.size()) {
		length = part.size() - offset;
	}
	auto res = part.read(offset, dest, length);

	// debug_i("read(0x%08x, 0x%08x): %u", offset, length, res);

	return res ? length : 0;
#endif
}

#endif
