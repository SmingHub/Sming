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
#include "StationListImpl.h"

AccessPointClass& WifiAccessPoint{SmingInternal::Network::accessPoint};

namespace SmingInternal
{
namespace Network
{
AccessPointImpl accessPoint;

void AccessPointImpl::enable(bool enabled, bool save)
{
	wifi_mode_t mode;
	ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
	if(enabled) {
		if(apNetworkInterface == nullptr) {
			apNetworkInterface = esp_netif_create_default_wifi_ap();
		}
		switch(mode) {
		case WIFI_MODE_AP:
		case WIFI_MODE_APSTA:
			return; // No change required
		case WIFI_MODE_STA:
			mode = WIFI_MODE_APSTA;
			break;
		case WIFI_MODE_NULL:
		default:
			mode = WIFI_MODE_AP;
		}
	} else {
		switch(mode) {
		case WIFI_MODE_NULL:
		case WIFI_MODE_STA:
			return; // No change required
		case WIFI_MODE_APSTA:
			mode = WIFI_MODE_STA;
			break;
		case WIFI_MODE_AP:
		default:
			mode = WIFI_MODE_NULL;
			break;
		}
		if(apNetworkInterface) {
			esp_netif_destroy(apNetworkInterface);
			apNetworkInterface = nullptr;
		}
	}
	ESP_ERROR_CHECK(esp_wifi_set_storage(save ? WIFI_STORAGE_FLASH : WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(mode));
}

bool AccessPointImpl::isEnabled() const
{
	uint8_t mode;
	ESP_ERROR_CHECK(esp_wifi_get_mode((wifi_mode_t*)&mode));
	return (mode == WIFI_MODE_AP) || (mode == WIFI_MODE_APSTA);
}

bool AccessPointImpl::config(const String& ssid, String password, WifiAuthMode mode, bool hidden, int channel,
							 int beaconInterval)
{
	wifi_config_t config{};

	if(ssid.length() >= sizeof(config.ap.ssid)) {
		return false;
	}
	if(password.length() >= sizeof(config.ap.password)) {
		return false;
	}

	memcpy(config.ap.ssid, ssid.c_str(), ssid.length());
	memcpy(config.ap.password, password.c_str(), password.length());
	config.ap.ssid_len = ssid.length();
	config.ap.ssid_hidden = hidden;
	config.ap.channel = channel;
	config.ap.beacon_interval = beaconInterval;
	config.ap.authmode = (wifi_auth_mode_t)mode;
	config.ap.max_connection = 8;

	bool enabled = isEnabled();
	enable(true, false);

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &config));
	if(enabled) {
		System.queueCallback(esp_wifi_start);
	}

	return true;
}

IpAddress AccessPointImpl::getIP() const
{
	IpAddress addr;
	esp_netif_ip_info_t info;
	if(esp_netif_get_ip_info(apNetworkInterface, &info) == ESP_OK) {
		addr = info.ip.addr;
	}
	return addr;
}

IpAddress AccessPointImpl::getNetworkBroadcast() const
{
	IpAddress addr;
	esp_netif_ip_info_t info;
	if(esp_netif_get_ip_info(apNetworkInterface, &info) == ESP_OK) {
		addr = info.ip.addr | ~info.netmask.addr;
	}
	return addr;
}

IpAddress AccessPointImpl::getNetworkMask() const
{
	IpAddress addr;
	esp_netif_ip_info_t info;
	if(esp_netif_get_ip_info(apNetworkInterface, &info) == ESP_OK) {
		addr = info.netmask.addr;
	}
	return addr;
}

IpAddress AccessPointImpl::getNetworkGateway() const
{
	IpAddress addr;
	esp_netif_ip_info_t info;
	if(esp_netif_get_ip_info(apNetworkInterface, &info) == ESP_OK) {
		addr = info.gw.addr;
	}
	return addr;
}

bool AccessPointImpl::setIP(IpAddress address)
{
	if(apNetworkInterface == nullptr) {
		return false;
	}
	esp_netif_dhcps_stop(apNetworkInterface);
	esp_netif_ip_info_t info;
	ESP_ERROR_CHECK(esp_netif_get_ip_info(apNetworkInterface, &info));
	info.gw.addr = address;
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	ESP_ERROR_CHECK(esp_netif_set_ip_info(apNetworkInterface, &info));
	ESP_ERROR_CHECK(esp_netif_dhcps_start(apNetworkInterface));
	return true;
}

MacAddress AccessPointImpl::getMacAddress() const
{
	MacAddress addr;
	ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_AP, &addr[0]));
	return addr;
}

bool AccessPointImpl::setMacAddress(const MacAddress& addr) const
{
	return esp_wifi_set_mac(WIFI_IF_AP, &const_cast<MacAddress&>(addr)[0]);
}

String AccessPointImpl::getSSID() const
{
	wifi_config_t config{};
	if(esp_wifi_get_config(WIFI_IF_AP, &config) != ESP_OK) {
		debug_w("Can't read station configuration!");
		return nullptr;
	}
	auto ssid = reinterpret_cast<const char*>(config.ap.ssid);
	debugf("SSID: %s", ssid);
	return ssid;
}

String AccessPointImpl::getPassword() const
{
	wifi_config_t config{};
	if(esp_wifi_get_config(WIFI_IF_AP, &config) != ESP_OK) {
		debug_w("Can't read station configuration!");
		return nullptr;
	}
	auto pwd = reinterpret_cast<const char*>(config.ap.password);
	debug_d("Pass: %s", pwd);
	return pwd;
}

std::unique_ptr<StationList> AccessPointImpl::getStations() const
{
	return std::unique_ptr<StationList>(new StationListImpl);
}

} // namespace Network
} // namespace SmingInternal
