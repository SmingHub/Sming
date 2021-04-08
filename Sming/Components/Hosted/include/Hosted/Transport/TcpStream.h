#include <Network/TcpServer.h>
#include <Data/Buffer/CircularBuffer.h>
#include "TStream.h"

namespace Hosted
{
namespace Transport
{
class CompoundTcpClientStream : public Stream
{
public:
	CompoundTcpClientStream(TcpClient& client) : client(client), cBuffer(1024)
	{
	}

	void setClient(TcpClient& client)
	{
		this->client = client;
	}

	bool push(const uint8_t* buffer, size_t size)
	{
		int written = cBuffer.write(buffer, size);
		return (written == size);
	}

	size_t readBytes(char* buffer, size_t length) override
	{
		return cBuffer.readBytes(buffer, length);
	}

	size_t write(const uint8_t* buffer, size_t size) override
	{
		if(client.send(reinterpret_cast<const char*>(buffer), size)) {
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
	}

private:
	CircularBuffer cBuffer;
	TcpClient& client;
};

class TcpStream : public TStream
{
protected:
	virtual bool process(TcpClient& client, char* data, int size) = 0;
};

} // namespace Transport

} // namespace Hosted
