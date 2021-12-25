#include <HostTests.h>

#include <Network/TcpClient.h>
#include <Network/TcpServer.h>
#include <Data/Stream/MemoryDataStream.h>
#include <Platform/Station.h>

class TcpClientTest : public TestGroup
{
public:
	TcpClientTest() : TestGroup(_F("TcpClient"))
	{
	}

	void execute() override
	{
		if(!WifiStation.isConnected()) {
			Serial.println("No network, skipping tests");
			return;
		}

		constexpr int port = 9876;
		String inputData = "This is very long and complex text that will be sent using multiple complicated streams.";

		// Tcp Server
		server = new TcpServer(
			[this](TcpClient& client, char* data, int size) -> bool {
				// on data
				return receivedData.concat(data, size);
			},
			[this, inputData](TcpClient& client, bool successful) {
				// on client close
				if(finished) {
					return;
				}
				REQUIRE(successful == true);
				REQUIRE(receivedData == inputData);
				finished = true;
				shutdown();
			});
		server->listen(port);
		server->setTimeOut(USHRT_MAX);   // disable connection timeout
		server->setKeepAlive(USHRT_MAX); // disable connection timeout

		// Tcp Client
		bool connected = client.connect(WifiStation.getIP(), port);
		debug_d("Connected: %d", connected);

		TEST_CASE("TcpClient::send stream")
		{
			size_t offset = 0;

			// Send text using bytes
			client.send(inputData.c_str(), 5);
			offset += 5;

			// send data using more bytes
			client.send(inputData.c_str() + offset, 7);
			offset += 7;

			// send data as stream
			auto stream1 = new MemoryDataStream();
			stream1->write(inputData.c_str() + offset, 3);
			client.send(stream1);
			offset += 3;
			client.commit();

			// more stream
			auto stream2 = new LimitedMemoryStream(4);
			stream2->write(reinterpret_cast<const uint8_t*>(inputData.c_str()) + offset, 4);
			client.send(stream2);
			offset += 4;

			// and finally the rest of the bytes
			String rest = inputData.substring(offset);
			client.send(rest.c_str(), rest.length());
			client.setTimeOut(1);

			pending();
		}
	}

	void shutdown()
	{
		server->shutdown();
		server = nullptr;
		timer.initializeMs<1000>([this]() { complete(); });
		timer.startOnce();
	}

private:
	String receivedData;
	TcpServer* server{nullptr};
	TcpClient client{false};
	Timer timer;
	volatile bool finished = false;
};

void REGISTER_TEST(TcpClient)
{
	registerGroup<TcpClientTest>();
}
