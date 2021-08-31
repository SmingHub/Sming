#include <SmingCore.h>
#include <Platform/Ethernet.h>

#include "config.h"

#if CONFIG_ETH_USE_SPI_ETHERNET
#include "driver/spi_master.h"
#endif // CONFIG_ETH_USE_SPI_ETHERNET

Ethernet ethernet;

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

	ethernet.begin();

	esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
	esp_netif_t* eth_netif = esp_netif_new(&cfg);
	// Set default handlers to process TCP/IP stuffs
	ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
	// Register user defined event handers
	ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

	eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
	eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
	phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;
	phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO;
#if CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
	mac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;
	mac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;
	esp_eth_mac_t* mac = esp_eth_mac_new_esp32(&mac_config);
#if CONFIG_EXAMPLE_ETH_PHY_IP101
	esp_eth_phy_t* phy = esp_eth_phy_new_ip101(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_RTL8201
	esp_eth_phy_t* phy = esp_eth_phy_new_rtl8201(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_LAN8720
	esp_eth_phy_t* phy = esp_eth_phy_new_lan8720(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_DP83848
	esp_eth_phy_t* phy = esp_eth_phy_new_dp83848(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_KSZ8041
	esp_eth_phy_t* phy = esp_eth_phy_new_ksz8041(&phy_config);
#endif
#elif CONFIG_ETH_USE_SPI_ETHERNET
	gpio_install_isr_service(0);
	spi_device_handle_t spi_handle = NULL;
	spi_bus_config_t buscfg = {
		.miso_io_num = CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO,
		.mosi_io_num = CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO,
		.sclk_io_num = CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
	};
	ESP_ERROR_CHECK(spi_bus_initialize(CONFIG_EXAMPLE_ETH_SPI_HOST, &buscfg, 1));
#if CONFIG_EXAMPLE_USE_DM9051
	spi_device_interface_config_t devcfg = {.command_bits = 1,
											.address_bits = 7,
											.mode = 0,
											.clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
											.spics_io_num = CONFIG_EXAMPLE_ETH_SPI_CS_GPIO,
											.queue_size = 20};
	ESP_ERROR_CHECK(spi_bus_add_device(CONFIG_EXAMPLE_ETH_SPI_HOST, &devcfg, &spi_handle));
	/* dm9051 ethernet driver is based on spi driver */
	eth_dm9051_config_t dm9051_config = ETH_DM9051_DEFAULT_CONFIG(spi_handle);
	dm9051_config.int_gpio_num = CONFIG_EXAMPLE_ETH_SPI_INT_GPIO;
	esp_eth_mac_t* mac = esp_eth_mac_new_dm9051(&dm9051_config, &mac_config);
	esp_eth_phy_t* phy = esp_eth_phy_new_dm9051(&phy_config);
#elif CONFIG_EXAMPLE_USE_W5500
	spi_device_interface_config_t devcfg = {.command_bits = 16, // Actually it's the address phase in W5500 SPI frame
											.address_bits = 8,  // Actually it's the control phase in W5500 SPI frame
											.mode = 0,
											.clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
											.spics_io_num = CONFIG_EXAMPLE_ETH_SPI_CS_GPIO,
											.queue_size = 20};
	ESP_ERROR_CHECK(spi_bus_add_device(CONFIG_EXAMPLE_ETH_SPI_HOST, &devcfg, &spi_handle));
	/* w5500 ethernet driver is based on spi driver */
	eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(spi_handle);
	w5500_config.int_gpio_num = CONFIG_EXAMPLE_ETH_SPI_INT_GPIO;
	esp_eth_mac_t* mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
	esp_eth_phy_t* phy = esp_eth_phy_new_w5500(&phy_config);
#endif
#endif // CONFIG_ETH_USE_SPI_ETHERNET
	esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
	esp_eth_handle_t eth_handle = NULL;
	ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
#if !CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
	/* The SPI Ethernet module might doesn't have a burned factory MAC address, we cat to set it manually.
       02:00:00 is a Locally Administered OUI range so should not be used except when testing on a LAN under your control.
    */
	ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, (uint8_t[]){0x02, 0x00, 0x00, 0x12, 0x34, 0x56}));
#endif
	/* attach Ethernet driver to TCP/IP stack */
	ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
	/* start Ethernet driver state machine */
	ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}
