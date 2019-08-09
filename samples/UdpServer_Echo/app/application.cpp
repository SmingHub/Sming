#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

void onReceive(UdpConnection& connection, char* data, int size, IpAddress remoteIP, uint16_t remotePort); // Declaration

// UDP server
const uint16_t EchoPort = 1234;
UdpConnection udp(onReceive);

void onReceive(UdpConnection& connection, char* data, int size, IpAddress remoteIP, uint16_t remotePort)
{
	debugf("UDP Server callback from %s:%d, %d bytes", remoteIP.toString().c_str(), remotePort, size);

	// We implement string mode server for example
	Serial.print(">\t");
	Serial.print(data);

	// Send echo to remote sender
	String text = String("echo: ") + data;
	udp.sendStringTo(remoteIP, remotePort, text);
}

void gotIP(IpAddress ip, IpAddress gateway, IpAddress netmask)
{
	udp.listen(EchoPort);

	Serial.println("\r\n=== UDP SERVER STARTED ===");
	Serial.print(WifiStation.getIP());
	Serial.print(":");
	Serial.println(EchoPort);
	Serial.println("=============================\r\n");
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true);

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiEvents.onStationGotIP(gotIP);
}
