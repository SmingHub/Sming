#include <HostTests.h>

#include <WHashMap.h>
#include <Digital.h>
#include <Network/TcpServer.h>
#include <Hosted/Client.h>
#include <Hosted/Util.h>
#include <Hosted/Transport/TcpServerTransport.h>
#include <Platform/Station.h>

namespace
{
using namespace simpleRPC;

static uint32_t plusCommand(uint8_t a, uint16_t b)
{
	return a + b;
};

class TheWire
{
public:
	void begin()
	{
		called++;
	}

	uint8_t begin(uint8_t a, uint8_t b)
	{
		called += (a + b);
		return called;
	}

	uint8_t getCalled()
	{
		return called;
	}

private:
	uint8_t called{0};
};

TheWire theWire;

class HostedTest : public TestGroup
{
public:
	using RemoteCommands = HashMap<String, uint8_t>;

	HostedTest() : TestGroup("Hosted")
	{
	}

	void execute() override
	{
		char packet[]{"simpleRPC\0" // protocol identifier
					  "\3\0\0"		// version
					  "<I\0"		// little-endian
					  ": H B;pinMode: Sets mode of digital pin. @pin: Pin number, @mode: Mode type.\0"
					  "B: H;digitalRead: Read digital pin. @pin: Pin number. @return: Pin value.\0"
					  ": H B;digitalWrite: Write to a digital pin. @pin: Pin number. @value: Pin value.\0"
					  "\0"};

		ParserSettings settings{
			.startMethods = ParserSettings::SimpleMethod(&HostedTest::startMethods, this),
			.startMethod = ParserSettings::SimpleMethod(&HostedTest::startMethod, this),
			.methodName = ParserSettings::CharMethod(&HostedTest::methodName, this),
			.endMethod = ParserSettings::SimpleMethod(&HostedTest::endMethod, this),
			.endMethods = ParserSettings::SimpleMethod(&HostedTest::endMethods, this),
			.state = ParserState::ready,
		};

		TEST_CASE("simpleRPC::parse()")
		{
			REQUIRE_EQ(parse(settings, packet, sizeof(packet)), ParserResult::finished);
			REQUIRE_EQ(commands.count(), 3);
			REQUIRE_EQ(commands["digitalWrite"], 2);
			REQUIRE_NEQ(commands["pinMode"], 2);
			REQUIRE_EQ(commands["pinMode"], 0);
		}

		if(!WifiStation.isConnected()) {
			Serial.println("No network, skipping tests");
			return;
		}

		// RPC Server
		auto server = new TcpServer();
		server->listen(4031);
		server->setTimeOut(USHRT_MAX);   // disable connection timeout
		server->setKeepAlive(USHRT_MAX); // disable connection timeout

		Hosted::Transport::TcpServerTransport transport(*server);
		transport.onData([](Stream& stream) {
			// clang-format off
				interface(stream,
					/*
					 * You can add more commands here. For every command you should specify command and text description in the format below.
					 * For more information read the SimpleRPC interface API: https://simplerpc.readthedocs.io/en/latest/api/interface.html
					 */
					pinMode, "pinMode> Sets mode of digital pin. @pin: Pin number, @mode: Mode type.",
					digitalRead, "digitalRead> Read digital pin. @pin: Pin number. @return: Pin value.",
					plusCommand, "plusCommand> Sum two numbers. @a: number one. @b: number two.",
					/* class methods */
					// uint8_t TwoWire::begin(uint8_t sda, uint8_t scl)
					makeTuple(&theWire, static_cast<uint8_t(TheWire::*)(uint8_t,uint8_t)>(&TheWire::begin)), "TheWire::begin> Starts two-wire communication. @sda: Data pin. @scl: Clock pin.",
					// void TheWire::begin()
					makeTuple(&theWire, static_cast<void(TheWire::*)()>(&TheWire::begin)), "TheWire::begin> Starts two-wire communication.",
					makeTuple(&theWire, &TheWire::getCalled), "TheWire::getCalled> Gets times called. @return: Result."
				);
			// clang-format on

			return true;
		});

		// RPC Client

		TcpClient client{false};
		client.connect(WifiStation.getIP(), 4031);
		Hosted::Transport::TcpClientStream stream(client, 1024);

		Hosted::Client hostedClient(stream, '>');

		TEST_CASE("Client::getRemoteCommands()")
		{
			REQUIRE(hostedClient.getRemoteCommands() == true);
			REQUIRE_EQ(hostedClient.getFunctionId("plusCommand"), 2);
			REQUIRE_EQ(hostedClient.getFunctionId("uint8_t TheWire::begin(uint8_t, uint8_t)"), 3);
		}

		TEST_CASE("Client::send and wait()")
		{
			ElapseTimer timer;

			REQUIRE(hostedClient.send("plusCommand", uint8_t(3), uint16_t(2)) == true);
			REQUIRE_EQ(hostedClient.wait<uint32_t>(), 5);

			debug_i("PlusCommand Roundtrip Time: %s", timer.elapsedTime().toString().c_str());
		}

		TEST_CASE("Client::send and check class method")
		{
			REQUIRE(hostedClient.send("uint8_t TheWire::begin(uint8_t, uint8_t)", uint8_t(3), uint8_t(3)) == true);
			REQUIRE_EQ(hostedClient.wait<uint8_t>(), 6);

			REQUIRE(hostedClient.send("uint8_t TheWire::getCalled()") == true);
			REQUIRE_EQ(hostedClient.wait<uint8_t>(), 6);
		}

		server->shutdown();
	}

private:
	RemoteCommands commands;
	uint8_t methodPosition = 0;
	String parsedCommand;

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

} // namespace

void REGISTER_TEST(Hosted)
{
	registerGroup<HostedTest>();
}
