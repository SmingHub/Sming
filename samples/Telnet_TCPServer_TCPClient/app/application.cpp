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
		Debug.printf("Heap change, current = %d\r\n", currentHeap);
		savedHeap = currentHeap;
	}
}

void applicationCommand(String commandLine, CommandOutput* commandOutput)
{
	commandOutput->printf("Hello from Telnet Example application\r\nYou entered : '");
	commandOutput->printf(commandLine.c_str());
	commandOutput->printf("'\r\n");
	commandOutput->printf("Tokenized commandLine is : \r\n");

	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);
	for(int i = 0; i < numToken; i++) {
		commandOutput->printf("%d : %s\r\n", i, commandToken.at(i).c_str());
	}
}

void appheapCommand(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);
	if(numToken != 2) {
		commandOutput->printf("Usage appheap on/off/now\r\n");
	} else if(commandToken[1] == "on") {
		commandOutput->printf("Timer heap display started \r\n");
		savedHeap = 0;
		memoryTimer.initializeMs(250, checkHeap).start();
	} else if(commandToken[1] == "off") {
		commandOutput->printf("Timer heap display stopped \r\n");
		savedHeap = 0;
		memoryTimer.stop();
	} else if(commandToken[1] == "now") {
		commandOutput->printf("Heap current free = %d\r\n", system_get_free_heap_size());
	} else {
		commandOutput->printf("Usage appheap on/off/now\r\n");
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
	client.sendString("sendString data\r\n", false);
	client.writeString("writeString data\r\n", 0);
	if(strcmp(data, "close") == 0) {
		debugf("Closing client");
		client.close();
	};
	return true;
}

void tcpServerClientComplete(TcpClient& client, bool succesfull)
{
	debugf("Application CompleteCallback : %s \r\n", client.getRemoteIp().toString().c_str());
}

TcpServer tcpServer(tcpServerClientConnected, tcpServerClientReceive, tcpServerClientComplete);
TelnetServer telnetServer;

void startServers()
{
	tcpServer.listen(8023);

	Serial.println("\r\n=== TCP SERVER Port 8023 STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");

	telnetServer.listen(23);

	Serial.println("\r\n=== Telnet SERVER Port 23 STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");

	commandHandler.registerCommand(CommandDelegate("application", "This command is defined by the application\r\n",
												   "testGroup", applicationCommand));
}

void connectFail(const String& ssid, const MACAddress& bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
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
	Debug.printf("This is the debug output\r\n");
	telnetServer.enableDebug(true); /* is default but here to show possibility */
	commandHandler.registerCommand(
		CommandDelegate("appheap", "Usage appheap on/off/now for heapdisplay\r\n", "testGroup", appheapCommand));
	memoryTimer.initializeMs(250, checkHeap).start();
}
