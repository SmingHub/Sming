/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AccessPointImpl.cpp
 *
 ****/

#include "AccessPointImpl.h"
#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_system.h>

static AccessPointImpl accessPoint;
AccessPointClass& WifiAccessPoint = accessPoint;

static esp_netif_t* apNetworkInterface = nullptr;

void AccessPointImpl::enable(bool enabled, bool save)
{
	uint8_t mode;
	esp_wifi_get_mode((wifi_mode_t*)&mode);
	if(enabled) {
		mode |= WIFI_MODE_AP;
		if(apNetworkInterface == nullptr) {
			apNetworkInterface = esp_netif_create_default_wifi_ap();
		}
	} else if(apNetworkInterface) {
		esp_netif_destroy(apNetworkInterface);
		apNetworkInterface = nullptr;
	}
	esp_wifi_set_storage(save ? WIFI_STORAGE_FLASH : WIFI_STORAGE_RAM);
	esp_wifi_set_mode((wifi_mode_t)mode);
}

bool AccessPointImpl::isEnabled() const
{
	uint8_t mode;
	esp_wifi_get_mode((wifi_mode_t*)&mode);
	return mode & WIFI_MODE_AP;
}

bool AccessPointImpl::config(const String& ssid, String password, WifiAuthMode mode, bool hidden, int channel,
							 int beaconInterval)
{
	wifi_config_t config;

	if(ssid.length() >= sizeof(config.ap.ssid)) {
		return false;
	}
	if(password.length() >= sizeof(config.ap.password)) {
		return false;
	}

	memcpy(&config.ap.ssid, ssid.c_str(), ssid.length());
	memcpy(&config.ap.password, password.c_str(), password.length());
	config.ap.ssid_len = ssid.length();
	config.ap.ssid_hidden = hidden;
	config.ap.channel = channel;
	config.ap.beacon_interval = beaconInterval;
	config.ap.authmode = (wifi_auth_mode_t)mode;
	config.ap.max_connection = 8;

	uint8_t wifiMode;
	esp_wifi_get_mode((wifi_mode_t*)&wifiMode);
	wifiMode |= WIFI_MODE_AP;
	ESP_ERROR_CHECK(esp_wifi_set_mode((wifi_mode_t)wifiMode));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &config));

	ESP_ERROR_CHECK(esp_wifi_start());

	return true;
}

IpAddress AccessPointImpl::getIP() const
{
	esp_netif_ip_info_t info;
	esp_netif_get_ip_info(apNetworkInterface, &info);
	return info.ip.addr;
}

IpAddress AccessPointImpl::getNetworkBroadcast() const
{
	esp_netif_ip_info_t info;
	esp_netif_get_ip_info(apNetworkInterface, &info);
	return (info.ip.addr | ~info.netmask.addr);
}

IpAddress AccessPointImpl::getNetworkMask() const
{
	esp_netif_ip_info_t info;
	esp_netif_get_ip_info(apNetworkInterface, &info);
	return info.netmask.addr;
}

IpAddress AccessPointImpl::getNetworkGateway() const
{
	esp_netif_ip_info_t info;
	esp_netif_get_ip_info(apNetworkInterface, &info);
	return info.gw.addr;
}

bool AccessPointImpl::setIP(IpAddress address)
{
	esp_netif_dhcps_stop(apNetworkInterface);
	esp_netif_ip_info_t info;
	esp_netif_get_ip_info(apNetworkInterface, &info);
	info.ip.addr = address;
	info.gw.addr = address;
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	esp_netif_set_ip_info(apNetworkInterface, &info);
	esp_netif_dhcps_start(apNetworkInterface);
	return true;
}

MacAddress AccessPointImpl::getMacAddress() const
{
	MacAddress addr;
	if(esp_wifi_get_mac(ESP_IF_WIFI_AP, (uint8_t*)&addr[0]) == ESP_OK) {
		return addr;
	} else {
		return MACADDR_NONE;
	}
}

String AccessPointImpl::getSSID() const
{
	wifi_config_t config = {0};
	if(esp_wifi_get_config(ESP_IF_WIFI_AP, &config) != ESP_OK) {
		debugf("Can't read station configuration!");
		return nullptr;
	}
	auto ssid = reinterpret_cast<const char*>(config.ap.ssid);
	debugf("SSID: %s", ssid);
	return ssid;
}

String AccessPointImpl::getPassword() const
{
	wifi_config_t config = {0};
	if(esp_wifi_get_config(ESP_IF_WIFI_AP, &config) != ESP_OK) {
		debugf("Can't read station configuration!");
		return nullptr;
	}
	auto pwd = reinterpret_cast<const char*>(config.ap.password);
	debugf("Pass: %s", pwd);
	return pwd;
}

void AccessPointImpl::onSystemReady()
{
}
