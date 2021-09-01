#include <SmingCore.h>

#ifdef SUBARCH_ESP32

#include <Platform/EmbeddedEthernet.h>
#include <Network/Ethernet/Lan8720.h>

Ethernet::Lan8720 phy;
EmbeddedEthernet ethernet(phy);

static void ethernetEventHandler(Ethernet::Event event, MacAddress mac)
{
	Serial.print(toString(event));
	if(mac) {
		Serial.print(_F(", MAC = "));
		Serial.print(mac.toString());
	}
	Serial.println();
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
	ethernet.onEvent(ethernetEventHandler);
	ethernet.onGotIp(ethernetGotIp);

	// Modify default config as required
	Ethernet::Config config;
	ethernet.begin(config);
}

#else

void init()
{
}

#endif
