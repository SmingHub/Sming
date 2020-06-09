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
#include <tcpip_adapter.h>
#include <esp_wifi.h>
#include <esp_system.h>

static AccessPointImpl accessPoint;
AccessPointClass& WifiAccessPoint = accessPoint;

void AccessPointImpl::enable(bool enabled, bool save)
{
	uint8_t mode;
	esp_wifi_get_mode((wifi_mode_t*)&mode);
	if(enabled) {
		mode |= WIFI_MODE_AP;
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
	tcpip_adapter_ip_info_t info;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &info);
	return info.ip;
}

IpAddress AccessPointImpl::getNetworkBroadcast() const
{
	tcpip_adapter_ip_info_t info;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &info);
	return (info.ip.addr | ~info.netmask.addr);
}

IpAddress AccessPointImpl::getNetworkMask() const
{
	tcpip_adapter_ip_info_t info;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &info);
	return info.netmask;
}

IpAddress AccessPointImpl::getNetworkGateway() const
{
	tcpip_adapter_ip_info_t info;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &info);
	return info.gw;
}

bool AccessPointImpl::setIP(IpAddress address)
{
	tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
	tcpip_adapter_ip_info_t info;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &info);
	info.ip = address;
	info.gw = address;
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info);
	tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
	return true;
}

MacAddress AccessPointImpl::getMacAddress() const
{
	MacAddress addr;
	if(esp_wifi_get_mac(WIFI_IF_AP, (uint8_t*)&addr[0]) == ESP_OK) {
		return addr;
	} else {
		return MACADDR_NONE;
	}
}

String AccessPointImpl::getSSID() const
{
	wifi_config_t config = {0};
	if(esp_wifi_get_config(WIFI_IF_AP, &config) != ESP_OK) {
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
	if(esp_wifi_get_config(WIFI_IF_AP, &config) != ESP_OK) {
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
