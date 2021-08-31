#include <SmingCore.h>
#include <Platform/InternalEthernet.h>
#include <Network/Ethernet/Lan8720.h>

InternalEthernet ethernet;

static void ethernetEventHandler(EthernetEvent event, MacAddress mac)
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

	Ethernet::PhyConfig config;
	config.phyAddr = 0;
	auto phy = new Lan8720(config);
	ethernet.begin(phy);
}
