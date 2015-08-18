#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Network/TelnetServer.h>
#include "Services/CommandProcessing/CommandProcessingIncludes.h"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

void applicationCommand(String commandLine  ,CommandOutput* commandOutput)
{
	commandOutput->printf("Hello from Telnet Example application\r\nYou entered : '");
	commandOutput->printf(commandLine.c_str());
	commandOutput->printf("'\r\n");
	commandOutput->printf("Tokenized commandLine is : \r\n");

	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ' , commandToken);
	for (int i=0;i<numToken;i++)
	{
		commandOutput->printf("%d : %s\r\n",i,commandToken.at(i).c_str());
	}
}

void tcpServerClientConnected (TcpClient* client)
{
	debugf("Application onClientCallback : %s\r\n",client->getRemoteIp().toString().c_str());
}

bool tcpServerClientReceive (TcpClient& client, char *data, int size)
{
	debugf("Application DataCallback : %s, %d bytes \r\n",client.getRemoteIp().toString().c_str(),size );
	debugf("Data : %s", data);
	client.sendString("sendString data\r\n", false);
	client.writeString("writeString data\r\n",0 );
	if (strcmp(data,"close") == 0)
	{
		debugf("Closing client");
		client.close();
	};
	return true;
}



void tcpServerClientComplete(TcpClient& client, bool succesfull)
{
	debugf("Application CompleteCallback : %s \r\n",client.getRemoteIp().toString().c_str() );
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

	commandHandler.registerCommand(CommandDelegate("application","This command is defined by the application\r\n","testGroup", applicationCommand));
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");
	startServers();
}

void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connectOk, 10, connectFail);
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
	WifiStation.waitConnection(connectOk, 30, connectFail);

}
