#include <SmingCore.h>

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

	   auto onReceive = [](TcpClient& client, char* data, int size)->bool {
		   	 return true;
	   };

	   client = new TcpClient(onCompleted, onReadyToSend, onReceive);
	}

	size_t write(const uint8_t* buffer, size_t size) override
	{
		if(client->getConnectionState() == eTCS_Ready) {
			client->connect(host, (int)port);
		}
		client->send((const char *)buffer, size);
		return size;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		return 0;
	}


	bool isFinished() override
	{
		return false;
	}


private:
	TcpClient* client =nullptr;
	String host;
	uint16_t port = 0;
};
