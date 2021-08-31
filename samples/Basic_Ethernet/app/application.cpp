#include <SmingCore.h>

#ifdef SUBARCH_ESP32

#include <Platform/EmbeddedEthernet.h>
#include <Network/Ethernet/Lan8720.h>

EmbeddedEthernet ethernet;

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
	Ethernet::MacConfig macConfig;
	Ethernet::PhyConfig phyConfig;
	auto phy = new Ethernet::Lan8720(phyConfig);
	ethernet.begin(macConfig, phy);
}

#else

void init()
{
}

#endif
