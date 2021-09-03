#include <SmingCore.h>

#ifdef SUBARCH_ESP32

#include <Platform/EmbeddedEthernet.h>
#include <Network/Ethernet/Lan8720.h>

Ethernet::Lan8720 phy;
EmbeddedEthernet ethernet(phy);

static void ethernetEventHandler(Ethernet::Event event)
{
	Serial.print(toString(event));
	Serial.print(_F(", MAC = "));
	Serial.println(ethernet.getMacAddress().toString());
}

static void ethernetGotIp(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.print(_F("Connected! Ethernet IP "));
	Serial.print(ip.toString());
	Serial.print(_F(", netmask "));
	Serial.print(netmask.toString());
	Serial.print(_F(", gateway "));
	Serial.println(gateway.toString());
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	ethernet.onEvent(ethernetEventHandler);
	ethernet.onGotIp(ethernetGotIp);

	// Modify default config as required
	Ethernet::Config config;
	ethernet.begin(config);

	// Change the advertised hostname for DHCP
	ethernet.setHostname("sming-ethernet");

	// Set a static IP
	// ethernet.setIP(IpAddress("192.168.1.12"), IpAddress("255.255.255.0"), IpAddress("192.168.1.254"));
}

#else

void init()
{
}

#endif
