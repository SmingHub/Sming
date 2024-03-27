#include <SmingCore.h>
#include <CommandProcessing/Utils.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
CommandProcessing::Handler commandHandler;

void clientConnected(TcpClient* client)
{
	Serial << _F("Client ") << client->getRemoteIp() << _F(" connected") << endl;
	if(commandHandler.isVerbose()) {
		client->sendString(commandHandler.getCommandWelcomeMessage());
		client->sendString(commandHandler.getCommandPrompt());
	}
}

void clientComplete(TcpClient& client, bool successful)
{
	Serial << _F("Client disconnected") << endl;
}

bool processTelnetInput(TcpClient& client, char* data, int size)
{
	// Ignore TELNET escape sequences
	constexpr char TC_ESC{'\xff'};
	uint8_t skip{};
	while(size-- > 0) {
		char c = *data++;
		if(skip) {
			--skip;
		} else if(c == '\xff') {
			skip = 2;
		} else {
			commandHandler.process(c);
		}
	}

	String s;
	commandHandler.getOutputStream().moveString(s);
	return s ? client.sendString(s) : true;
}

void processExampleCommand(String commandLine, ReadWriteStream& commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);

	if(numToken == 1) {
		commandOutput.println("example: No parameters provided");
		return;
	}

	commandOutput.printf("example: %d parameters provided\r\n", numToken);
}

void initCommands()
{
	commandHandler.registerSystemCommands();
	commandHandler.registerCommand({CMDP_STRINGS("example", "Example Command", "Application"), processExampleCommand});
}

TcpServer telnetServer(clientConnected, processTelnetInput, clientComplete);

// Will be called when station is fully operational
void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	telnetServer.listen(23);
	Serial.println(_F("\r\n=== TelnetServer SERVER STARTED ==="));
	Serial.println(_F("==============================\r\n"));
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial
	commandHandler.setVerbose(true);

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	WifiEvents.onStationGotIP(gotIP);

	// set command handlers
	initCommands();
}
