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

namespace SmingInternal::Network
{
AccessPointImpl accessPoint;

void AccessPointImpl::enable(bool enabled, bool save)
{
	if(enabled != this->enabled) {
		cyw43_wifi_set_up(&cyw43_state, CYW43_ITF_AP, enabled, cyw43_arch_get_country_code());
		if(enabled) {
			dhcps_start();
		} else {
			dhcps_stop();
		}
		this->enabled = enabled;
	}
}

bool AccessPointImpl::isEnabled() const
{
	return enabled;
}

static unsigned translateAuthMode(WifiAuthMode mode)
{
	// See cyw43_ll.h and whd_types.h for more details
	switch(mode) {
	case AUTH_OPEN:
		return CYW43_AUTH_OPEN;
	case AUTH_WPA_PSK:
		return CYW43_AUTH_WPA_TKIP_PSK;
	case AUTH_WPA_WPA2_PSK:
		return CYW43_AUTH_WPA2_MIXED_PSK;
	case AUTH_WEP:
	case AUTH_WPA2_PSK:
	default:
		return CYW43_AUTH_WPA2_AES_PSK;
	}
}

bool AccessPointImpl::config(const String& ssid, String password, WifiAuthMode mode, bool hidden, int channel,
							 int beaconInterval)
{
	unsigned auth = translateAuthMode(mode);
	cyw43_wifi_ap_set_ssid(&cyw43_state, ssid.length(), reinterpret_cast<const uint8_t*>(ssid.c_str()));
	if(password) {
		cyw43_wifi_ap_set_password(&cyw43_state, password.length(), reinterpret_cast<const uint8_t*>(password.c_str()));
		cyw43_wifi_ap_set_auth(&cyw43_state, auth);
	} else {
		cyw43_wifi_ap_set_auth(&cyw43_state, CYW43_AUTH_OPEN);
	}
	if(enabled) {
		cyw43_wifi_set_up(&cyw43_state, CYW43_ITF_AP, true, cyw43_arch_get_country_code());
		dhcps_start();
	}
	return true;
}

IpAddress AccessPointImpl::getIP() const
{
	auto netif = &cyw43_state.netif[CYW43_ITF_AP];
	return *netif_ip_addr4(netif);
}

IpAddress AccessPointImpl::getNetworkBroadcast() const
{
	auto netif = &cyw43_state.netif[CYW43_ITF_AP];
	return netif_ip_addr4(netif)->addr | ~netif_ip_netmask4(netif)->addr;
}

IpAddress AccessPointImpl::getNetworkMask() const
{
	auto netif = &cyw43_state.netif[CYW43_ITF_AP];
	return *netif_ip_netmask4(netif);
}

IpAddress AccessPointImpl::getNetworkGateway() const
{
	auto netif = &cyw43_state.netif[CYW43_ITF_AP];
	return *netif_ip_gw4(netif);
}

bool AccessPointImpl::setIP(IpAddress address)
{
	auto netif = &cyw43_state.netif[CYW43_ITF_AP];
	netif_set_ipaddr(netif, address);
	dhcps_start();
	return true;
}

void AccessPointImpl::dhcps_start()
{
	auto netif = &cyw43_state.netif[CYW43_ITF_AP];
	ip4_addr_t gw = *netif_ip_addr4(netif);
	ip4_addr_t mask;
	IP4_ADDR(&mask, 255, 255, 255, 0);
	dhcp_server_init(&dhcp_server, &gw, &mask);
}

void AccessPointImpl::dhcps_stop()
{
	dhcp_server_deinit(&dhcp_server);
}

MacAddress AccessPointImpl::getMacAddress() const
{
	MacAddress mac;
	cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_AP, &mac[0]);
	return mac;
}

bool AccessPointImpl::setMacAddress(const MacAddress& addr) const
{
	// Not implemented
	return false;
}

String AccessPointImpl::getSSID() const
{
	return String(reinterpret_cast<const char*>(cyw43_state.ap_ssid), cyw43_state.ap_ssid_len);
}

String AccessPointImpl::getPassword() const
{
	return String(reinterpret_cast<const char*>(cyw43_state.ap_key), cyw43_state.ap_key_len);
}

std::unique_ptr<StationList> AccessPointImpl::getStations() const
{
	return std::unique_ptr<StationList>(new StationListImpl);
}

void AccessPointImpl::onSystemReady()
{
}

void AccessPointImpl::eventHandler(EventInfo& info)
{
}

} // namespace SmingInternal::Network
