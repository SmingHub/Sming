#include <SmingCore.h>

// #define USE_EMBEDDED ARCH_ESP32

#if USE_EMBEDDED

// The Embedded MAC
#include <Platform/EmbeddedEthernet.h>

// PHY: See `Sming/Components/Network/src/Network/Ethernet` for others
#include <Network/Ethernet/Lan8720.h>

Ethernet::Lan8720 phy;
EmbeddedEthernet ethernet(phy);

#else

// Use the HardwareSPI library to initialise SPI bus
#include <HSPI/Controller.h>
HSPI::Controller spi;

// The Wiznet W5500 controller with integrated PHY
#include <Network/Ethernet/W5500.h>
Ethernet::W5500Service ethernet;

// #include <Network/Ethernet/DM9051.h>
// Ethernet::DM9051Service ethernet;

#endif

static void ethernetEventHandler(Ethernet::Event event)
{
	Serial << event << _F(", MAC = ") << ethernet.getMacAddress() << endl;
}

static void ethernetGotIp(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial << _F("Connected! Ethernet IP ") << ip << _F(", netmask ") << netmask << _F(", gateway ") << gateway << endl;
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	ethernet.onEvent(ethernetEventHandler);
	ethernet.onGotIp(ethernetGotIp);

#if USE_EMBEDDED
	// Modify default config as required
	EmbeddedEthernet::Config config;
	ethernet.begin(config);
#else
	if(!spi.begin()) {
		return;
	}
	Ethernet::SpiService::Config config;
	config.spiHost = spi.getHost();
	if(!ethernet.begin(config)) {
		return;
	}
#endif

	// Change the advertised hostname for DHCP
	ethernet.setHostname("sming-ethernet");

	// Set a static IP
	// ethernet.setIP(IpAddress("192.168.1.12"), IpAddress("255.255.255.0"), IpAddress("192.168.1.254"));
}
