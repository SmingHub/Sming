#include <SmingCore.h>

// #define USE_EMBEDDED ARCH_ESP32

#ifdef ARCH_ESP32

#if USE_EMBEDDED

#include <Platform/EmbeddedEthernet.h>
#include <Network/Ethernet/Lan8720.h>

Ethernet::Lan8720 phy;
EmbeddedEthernet ethernet(phy);

#else

#include <Network/Ethernet/W5500.h>
Ethernet::W5500Service ethernet;

#include "driver/spi_master.h"

#if defined(SUBARCH_ESP32)
#define SPI_PIN_MISO 25
#define SPI_PIN_MOSI 23
#define SPI_PIN_SCLK 19
#elif defined(SUBARCH_ESP32S2)
#define SPI_PIN_MISO 37
#define SPI_PIN_MOSI 35
#define SPI_PIN_SCLK 36
#elif defined(SUBARCH_ESP32C3)
#define SPI_PIN_MISO 2
#define SPI_PIN_MOSI 7
#define SPI_PIN_SCLK 6
#endif

#endif

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

#if USE_EMBEDDED
	// Modify default config as required
	EmbeddedEthernet::Config config;
	ethernet.begin(config);
#else
	spi_bus_config_t buscfg = {
		.mosi_io_num = SPI_PIN_MOSI,
		.miso_io_num = SPI_PIN_MISO,
		.sclk_io_num = SPI_PIN_SCLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.intr_flags = ESP_INTR_FLAG_IRAM,
	};
	auto err = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
	if(ESP_ERROR_CHECK_WITHOUT_ABORT(err) != ESP_OK) {
		return;
	}

	Ethernet::W5500Service::Config config;
	config.spiHost = SPI_HOST;
	if(!ethernet.begin(config)) {
		return;
	}
#endif

	// Change the advertised hostname for DHCP
	ethernet.setHostname("sming-ethernet");

	// Set a static IP
	// ethernet.setIP(IpAddress("192.168.1.12"), IpAddress("255.255.255.0"), IpAddress("192.168.1.254"));
}

#else // ARCH_ESP32

void init()
{
}

#endif
