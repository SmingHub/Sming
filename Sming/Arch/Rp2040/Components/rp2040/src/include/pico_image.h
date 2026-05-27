/****
 * @brief Support for accessing image definition structures
 */

#include <boot/picobin.h>

namespace Pico
{
// Range where IMAGE_DEF is expected at start of partition
constexpr uint32_t IMAGEDEF_START = 0x110;
constexpr uint32_t IMAGEDEF_END = 0x200;
constexpr uint32_t IMAGEDEF_BUFFER_SIZE = IMAGEDEF_END - IMAGEDEF_START;

struct ImageDefInfo {
	uint32_t* imageType;
	uint32_t* version;

	explicit operator bool() const
	{
		return imageType && version;
	}

	uint16_t getFlags() const
	{
		return imageType ? (imageType[0] >> 16) : 0;
	}

	void setFlags(uint16_t flags)
	{
		if(imageType) {
			*imageType = (*imageType & 0xffff) | (flags << 16);
		}
	}

	uint32_t getVersion() const
	{
		return version ? version[1] : 0;
	}

	void setVersion(uint32_t value) const
	{
		if(version) {
			version[1] = value;
		}
	}

	uint16_t getVersionMajor() const
	{
		return getVersion() >> 16;
	}

	uint16_t getVersionMinor() const
	{
		return getVersion() & 0xffff;
	}
};

ImageDefInfo findImageDef(void* buffer);

} // namespace Pico
