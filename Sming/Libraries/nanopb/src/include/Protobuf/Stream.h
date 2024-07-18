/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Stream.h
 *
 ****/

#pragma once

#include <WString.h>
#include <Data/Stream/DataSourceStream.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include <Network/TcpClient.h>

namespace Protobuf
{
class Stream
{
public:
	~Stream()
	{
	}
};

class InputStream : public Stream
{
public:
	InputStream(IDataSourceStream& source) : stream(source)
	{
	}

	bool decode(const pb_msgdesc_t* fields, void* dest_struct);

	String getErrorString() const
	{
		return errmsg;
	}

private:
	const char* errmsg{nullptr};
	IDataSourceStream& stream;
};

class OutputStream : public Stream
{
public:
	size_t encode(const pb_msgdesc_t* fields, const void* src_struct);

protected:
	static bool buf_write(pb_ostream_t* stream, const pb_byte_t* buf, size_t count)
	{
		auto self = static_cast<OutputStream*>(stream->state);
		assert(self != nullptr);
		return self->write(buf, count);
	}
	virtual bool write(const pb_byte_t* buf, size_t count) = 0;
};

class DummyOutputStream : public OutputStream
{
protected:
	bool write(const pb_byte_t*, size_t) override
	{
		return true;
	}
};

class TcpClientOutputStream : public OutputStream
{
public:
	TcpClientOutputStream(TcpClient& client) : client(client)
	{
	}

protected:
	bool write(const pb_byte_t* buf, size_t count) override
	{
		int err = client.send(reinterpret_cast<const char*>(buf), count);
		// debug_i("client.send(%u): %d", count, err);
		return err > 0;
	}

	TcpClient& client;
};

} // namespace Protobuf
