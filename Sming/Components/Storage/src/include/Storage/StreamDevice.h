/*
 * StreamDevice.h
 */

#include "Device.h"
#include <Data/Stream/DataSourceStream.h>

namespace Storage
{
/**
 * @brief Read-only partition on a stream object
 * @note Writes not possible as streams always append data, cannot do random writes
 */
class StreamDevice : public Device
{
public:
	StreamDevice(IDataSourceStream* stream, size_t size) : Device(nameOf(stream), size), mStream(stream)
	{
	}

	StreamDevice(IDataSourceStream* stream) : StreamDevice(stream, size_t(stream->available()))
	{
	}

	static String nameOf(IDataSourceStream* stream)
	{
		String s;
		if(stream != nullptr) {
			s = stream->getName();
		}
		if(!s) {
			s = F("stream_") + String(uint32_t(stream), HEX);
		}
		return s;
	}

	Type getType() const override
	{
		return Type::stream;
	}

	bool read(storage_size_t address, void* buffer, size_t len) override
	{
		if(mStream == nullptr) {
			return false;
		}
		if(storage_size_t(mStream->seekFrom(address, SeekOrigin::Start)) != address) {
			return false;
		}
		return mStream->readBytes(static_cast<char*>(buffer), len) == len;
	}

	bool write(storage_size_t address, const void* data, size_t len) override
	{
		return false;
	}

	bool erase_range(storage_size_t address, storage_size_t len) override
	{
		return false;
	}

private:
	std::unique_ptr<IDataSourceStream> mStream;
};

} // namespace Storage
