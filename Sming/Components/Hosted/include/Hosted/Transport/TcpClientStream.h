/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TcpClientStream.h
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#pragma once

#include <Network/TcpServer.h>
#include <Data/Buffer/CircularBuffer.h>

namespace Hosted::Transport
{
class TcpClientStream : public Stream
{
public:
	TcpClientStream(TcpClient& client, size_t cbufferSize = 1024, size_t threshold = 400)
		: cBuffer(cbufferSize), client(client), threshold(threshold)
	{
		client.setReceiveDelegate(TcpClientDataDelegate(&TcpClientStream::store, this));
	}

	void setClient(TcpClient& client)
	{
		this->client = client;
	}

	bool push(const uint8_t* buffer, size_t size)
	{
		size_t written = cBuffer.write(buffer, size);
		return (written == size);
	}

	size_t readBytes(char* buffer, size_t length) override
	{
		return cBuffer.readBytes(buffer, length);
	}

	size_t write(const uint8_t* buffer, size_t size) override
	{
		if(client.send(reinterpret_cast<const char*>(buffer), size)) {
			pendingBytes += size;
			if(pendingBytes > threshold) {
				pendingBytes = 0;
				client.commit();
			}
			return size;
		}

		return 0;
	}

	size_t write(uint8_t c) override
	{
		return cBuffer.write(c);
	}

	int available() override
	{
		return cBuffer.available();
	}

	int peek() override
	{
		return cBuffer.peek();
	}

	int read() override
	{
		return cBuffer.read();
	}

	void flush() override
	{
		client.commit();
	}

private:
	CircularBuffer cBuffer;
	TcpClient& client;
	size_t pendingBytes{0};
	size_t threshold;

	bool store(TcpClient&, char* data, int size)
	{
		return push(reinterpret_cast<const uint8_t*>(data), size);
	}
};

} // namespace Hosted::Transport
