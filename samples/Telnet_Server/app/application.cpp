#include <SmingCore.h>
#include <Network/TelnetServer.h>
#include "Services/CommandProcessing/CommandProcessingIncludes.h"
#include <Debug.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

Timer memoryTimer;
int savedHeap = 0;

void checkHeap()
{
	int currentHeap = system_get_free_heap_size();
	if(currentHeap != savedHeap) {
		Debug.printf(_F("Heap change, current = %u\r\n"), currentHeap);
		savedHeap = currentHeap;
	}
}

void applicationCommand(String commandLine, CommandOutput* commandOutput)
{
	commandOutput->print(_F("Hello from Telnet Example application\r\nYou entered : '"));
	commandOutput->print(commandLine.c_str());
	commandOutput->println('\'');
	commandOutput->println(_F("Tokenized commandLine is : "));

	Vector<String> commandToken;
	unsigned numToken = splitString(commandLine, ' ', commandToken);
	for(unsigned i = 0; i < numToken; i++) {
		commandOutput->printf(_F("%u : %s\r\n"), i, commandToken.at(i).c_str());
	}
}

void appheapCommand(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);
	if(numToken != 2) {
		commandOutput->println(_F("Usage appheap on/off/now"));
	} else if(commandToken[1] == "on") {
		commandOutput->println(_F("Timer heap display started"));
		savedHeap = 0;
		memoryTimer.initializeMs(250, checkHeap).start();
	} else if(commandToken[1] == "off") {
		commandOutput->println(_F("Timer heap display stopped"));
		savedHeap = 0;
		memoryTimer.stop();
	} else if(commandToken[1] == "now") {
		commandOutput->printf(_F("Heap current free = %u\r\n"), system_get_free_heap_size());
	} else {
		commandOutput->println(_F("Usage appheap on/off/now"));
	}
}

void tcpServerClientConnected(TcpClient* client)
{
	debugf("Application onClientCallback : %s\r\n", client->getRemoteIp().toString().c_str());
}

bool tcpServerClientReceive(TcpClient& client, char* data, int size)
{
	debugf("Application DataCallback : %s, %d bytes \r\n", client.getRemoteIp().toString().c_str(), size);
	debugf("Data : %s", data);
	client.sendString(F("sendString data\r\n"), false);
	client.writeString(F("writeString data\r\n"), 0);
	if(strcmp(data, "close") == 0) {
		debugf("Closing client");
		client.close();
	};
	return true;
}

void tcpServerClientComplete(TcpClient& client, bool successful)
{
	debugf("Application CompleteCallback : %s \r\n", client.getRemoteIp().toString().c_str());
}

TcpServer tcpServer(tcpServerClientConnected, tcpServerClientReceive, tcpServerClientComplete);
TelnetServer telnetServer;

void startServers()
{
	tcpServer.listen(8023);

	Serial.println(_F("\r\n=== TCP SERVER Port 8023 STARTED ==="));
	Serial.println(WifiStation.getIP());
	Serial.println(_F("==============================\r\n"));

	telnetServer.listen(23);

	Serial.println(_F("\r\n=== Telnet SERVER Port 23 STARTED ==="));
	Serial.println(WifiStation.getIP());
	Serial.println(_F("==============================\r\n"));

	commandHandler.registerCommand(CommandDelegate(
		F("application"), F("This command is defined by the application\r\n"), F("testGroup"), applicationCommand));
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	startServers();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial
	Serial.commandProcessing(true);
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
	Debug.setDebug(Serial);
	Debug.initCommand();
	Debug.start();
	Debug.println(_F("This is the debug output"));
	telnetServer.enableDebug(true); /* is default but here to show possibility */
	commandHandler.registerCommand(CommandDelegate(F("appheap"), F("Usage appheap on/off/now for heapdisplay\r\n"),
												   F("testGroup"), appheapCommand));
	memoryTimer.initializeMs(250, checkHeap).start();
}
