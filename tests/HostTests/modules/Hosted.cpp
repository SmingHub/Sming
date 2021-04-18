#include <HostTests.h>

#include <WHashMap.h>
#include <Digital.h>
#include <Network/TcpServer.h>
#include <Hosted/Client.h>
#include <Hosted/Transport/TcpServerTransport.h>
#include <Platform/Station.h>

using namespace simpleRPC;

static uint32_t plusCommand(uint8_t a, uint16_t b)
{
	return a + b;
};

class HostedTest : public TestGroup
{
public:
	using RemoteCommands = HashMap<String, uint8_t>;

	HostedTest() : TestGroup(_F("Hosted"))
	{
	}

	void execute() override
	{
		char packet[] = {
			0x73, 0x69, 0x6d, 0x70, 0x6c, 0x65, 0x52, 0x50, 0x43, 0x00, 0x03, 0x00, 0x00, 0x3c, 0x49, 0x00, 0x3a, 0x20,
			0x48, 0x20, 0x42, 0x3b, 0x70, 0x69, 0x6e, 0x4d, 0x6f, 0x64, 0x65, 0x3a, 0x20, 0x53, 0x65, 0x74, 0x73, 0x20,
			0x6d, 0x6f, 0x64, 0x65, 0x20, 0x6f, 0x66, 0x20, 0x64, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6c, 0x20, 0x70, 0x69,
			0x6e, 0x2e, 0x20, 0x40, 0x70, 0x69, 0x6e, 0x3a, 0x20, 0x50, 0x69, 0x6e, 0x20, 0x6e, 0x75, 0x6d, 0x62, 0x65,
			0x72, 0x2c, 0x20, 0x40, 0x6d, 0x6f, 0x64, 0x65, 0x3a, 0x20, 0x4d, 0x6f, 0x64, 0x65, 0x20, 0x74, 0x79, 0x70,
			0x65, 0x2e, 0x00, 0x42, 0x3a, 0x20, 0x48, 0x3b, 0x64, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6c, 0x52, 0x65, 0x61,
			0x64, 0x3a, 0x20, 0x52, 0x65, 0x61, 0x64, 0x20, 0x64, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6c, 0x20, 0x70, 0x69,
			0x6e, 0x2e, 0x20, 0x40, 0x70, 0x69, 0x6e, 0x3a, 0x20, 0x50, 0x69, 0x6e, 0x20, 0x6e, 0x75, 0x6d, 0x62, 0x65,
			0x72, 0x2e, 0x20, 0x40, 0x72, 0x65, 0x74, 0x75, 0x72, 0x6e, 0x3a, 0x20, 0x50, 0x69, 0x6e, 0x20, 0x76, 0x61,
			0x6c, 0x75, 0x65, 0x2e, 0x00, 0x3a, 0x20, 0x48, 0x20, 0x42, 0x3b, 0x64, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6c,
			0x57, 0x72, 0x69, 0x74, 0x65, 0x3a, 0x20, 0x57, 0x72, 0x69, 0x74, 0x65, 0x20, 0x74, 0x6f, 0x20, 0x61, 0x20,
			0x64, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6c, 0x20, 0x70, 0x69, 0x6e, 0x2e, 0x20, 0x40, 0x70, 0x69, 0x6e, 0x3a,
			0x20, 0x50, 0x69, 0x6e, 0x20, 0x6e, 0x75, 0x6d, 0x62, 0x65, 0x72, 0x2e, 0x20, 0x40, 0x76, 0x61, 0x6c, 0x75,
			0x65, 0x3a, 0x20, 0x50, 0x69, 0x6e, 0x20, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x2e, 0x00, 0x00};

		ParserSettings settings;
		settings.startMethods = ParserSettings::SimpleMethod(&HostedTest::startMethods, this);
		settings.startMethod = ParserSettings::SimpleMethod(&HostedTest::startMethod, this);
		settings.methodName = ParserSettings::CharMethod(&HostedTest::methodName, this);
		settings.endMethod = ParserSettings::SimpleMethod(&HostedTest::endMethod, this);
		settings.endMethods = ParserSettings::SimpleMethod(&HostedTest::endMethods, this);
		settings.state = ParserState::ready;

		TEST_CASE("simpleRPC::parse()")
		{
			REQUIRE(parse(settings, packet, sizeof(packet)) == ParserResult::finished);
			REQUIRE(commands.count() == 3);
			REQUIRE(commands["digitalWrite"] == 2);
			REQUIRE(commands["pinMode"] != 2);
			REQUIRE(commands["pinMode"] == 0);
		}

		if(!WifiStation.isConnected()) {
			Serial.println("No network, skipping tests");
			return;
		}

		// RPC Server
		server = new TcpServer();
		server->listen(4031);
		server->setTimeOut(USHRT_MAX);   // disable connection timeout
		server->setKeepAlive(USHRT_MAX); // disable connection timeout

		Hosted::Transport::TcpServerTransport transport(*server);
		transport.onData([](Stream& stream) {
			// clang-format off
				interface(stream,
					/*
					 * Below we are exporting the following remote commands:
					 * - pinMode
					 * - digitalRead
					 * - digitalWrite
					 * You can add more commands here. For every command you should specify command and text description in the format below.
					 * For more information read the SimpleRPC interface API: https://simplerpc.readthedocs.io/en/latest/api/interface.html
					 */
					pinMode, "pinMode: Sets mode of digital pin. @pin: Pin number, @mode: Mode type.",
					digitalRead, "digitalRead: Read digital pin. @pin: Pin number. @return: Pin value.",
					plusCommand, "plusCommand: Sum two numbers. @a: number one. @b: number two."
				);
			// clang-format on

			return true;
		});

		// RCP Client

		client.connect(WifiStation.getIP(), 4031);
		Hosted::Transport::TcpClientStream stream(client, 1024);

		Hosted::Client hostedClient(stream);

		TEST_CASE("Client::getRemoteCommands()")
		{
			REQUIRE(hostedClient.getRemoteCommands() == true);
			REQUIRE(hostedClient.getFunctionId("plusCommand") == 2);
		}

		TEST_CASE("Client::send and wait()")
		{

			ElapseTimer timer;

			REQUIRE(hostedClient.send("plusCommand", uint8_t(3), uint16_t(2)) == true);
			REQUIRE(hostedClient.wait<uint32_t>() == 5);

			debug_i("PlusCommand Roundtrip Time: %s", timer.elapsedTime().toString().c_str());
		}
	}

private:
	RemoteCommands commands;
	uint8_t methodPosition = 0;
	String parsedCommand;

	TcpServer* server{nullptr};
	TcpClient client{false};
	Hosted::Transport::TcpClientStream* stream{nullptr};

	void startMethods()
	{
		methodPosition = 0;
		commands.clear();
	}

	void startMethod()
	{
		parsedCommand = "";
	}

	void methodName(char ch)
	{
		parsedCommand += ch;
	}

	void endMethod()
	{
		commands[parsedCommand] = methodPosition++;
	}

	void endMethods()
	{
	}
};

void REGISTER_TEST(Hosted)
{
	registerGroup<HostedTest>();
}
