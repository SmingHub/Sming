#include <SmingCore.h>
#include <Data/Buffer/CircularBuffer.h>

class HostedTcpStream: public ReadWriteStream
{
public:
	HostedTcpStream(const String& host, uint16_t port): host(host), port(port)
	{
	   auto onCompleted = [](TcpClient& client, bool successful) {
			// onCompleted;

	   };

	   auto onReadyToSend = [](TcpClient& client, TcpConnectionEvent sourceEvent) {

	   };

	   auto onReceive = [this](TcpClient& client, char* data, int size)->bool {
		   	 size_t written = this->buffer.write((const uint8_t*)data, size);
		   	 return (written == (size_t)size);
	   };

	   client = new TcpClient(onCompleted, onReadyToSend, onReceive);
	}

	size_t write(const uint8_t* buffer, size_t size) override
	{
		if(!client->isProcessing()) {
			client->connect(host, (int)port);
		}

		if(!client->send((const char *)buffer, size)) {
			return 0;
		}

		return size;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		return buffer.readMemoryBlock(data, bufSize);
	}

	bool seek(int len) override
	{
		return buffer.seek(len);
	}


	bool isFinished() override
	{
		return false;
	}

	void flush() override
	{
		client->commit();
	}


private:
	TcpClient* client = nullptr;
	CircularBuffer buffer = CircularBuffer(1024);
	String host;
	uint16_t port = 0;
};
