#include "include/pico_image.h"

namespace Pico
{
/*
	Expected IMAGE_DEF block:

	d3 de ff ff		PICOBIN_BLOCK_MARKER_START
	42 01 21 10		PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE
						Flags: 0x1021
	48 02 00 00		PICOBIN_BLOCK_ITEM_1BS_VERSION
	06 00 01 00			Major 0x0001, Minor 0x0006
	ff 03 00 00		PICOBIN_BLOCK_ITEM_2BS_LAST
	a4 86 00 00			relative pointer to next block loop
	79 35 12 ab			PICOBIN_BLOCK_MARKER_END
*/
ImageDefInfo findImageDef(void* buffer)
{
	auto words = static_cast<uint32_t*>(buffer);
	unsigned offset = 0;
	const unsigned maxOffset = IMAGEDEF_BUFFER_SIZE / sizeof(uint32_t);
	while(words[offset++] != PICOBIN_BLOCK_MARKER_START) {
		if(offset >= maxOffset) {
			return {};
		}
	}

	ImageDefInfo info{};
	while(offset < maxOffset) {
		uint32_t w = words[offset];
		uint16_t len;
		if(w & 0x80) {
			len = (w >> 8) & 0xffff;
		} else {
			len = (w >> 8) & 0xff;
		}
		switch(w & 0xff) {
		case PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE:
			info.imageType = &words[offset];
			break;
		case PICOBIN_BLOCK_ITEM_1BS_VERSION:
			info.version = &words[offset];
			break;
		case PICOBIN_BLOCK_ITEM_2BS_LAST:
			return info;
		}
		offset += len;
	}

	// Block not found or malformed
	return {};
}

} // namespace Pico
