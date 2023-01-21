/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StationImpl.cpp
 *
 ****/

#include "StationImpl.h"
#include "WifiEventsImpl.h"
#include "whd.h"
#include <lwip/prot/dhcp.h>

#include <debug_progmem.h>

#define SCAN_TIMEOUT_MS 30000

StationClass& WifiStation{SmingInternal::Network::station};

namespace SmingInternal::Network
{
BssList StationImpl::scanResults;
SimpleTimer StationImpl::scanTimer;

StationImpl station;

void StationImpl::enable(bool enabled, bool save)
{
	debug_i("%s", __PRETTY_FUNCTION__);
	if(enabled != this->enabled) {
		cyw43_wifi_set_up(&cyw43_state, CYW43_ITF_STA, enabled, cyw43_arch_get_country_code());
		this->enabled = enabled;
	}
}

bool StationImpl::isEnabled() const
{
	return this->enabled;
}

bool StationImpl::config(const Config& cfg)
{
	this->cfg = cfg;
	return enabled ? internalConnect() : true;
}

bool StationImpl::connect()
{
	if(cfg.ssid.length() == 0) {
		return false;
	}

	int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
	debug_i("link_status %d", link_status);
	switch(link_status) {
	case CYW43_LINK_JOIN:
		return true;
	case CYW43_LINK_NOIP:
		cyw43_cb_tcpip_set_link_up(&cyw43_state, CYW43_ITF_STA);
		return true;
	case CYW43_LINK_UP:
		return true;
	case CYW43_LINK_FAIL:
	case CYW43_LINK_NONET:
	case CYW43_LINK_BADAUTH:
	case CYW43_LINK_DOWN:
		return internalConnect();
	default:
		return false;
	}
}

bool StationImpl::internalConnect()
{
	unsigned auth = cfg.password.length() ? CYW43_AUTH_WPA2_AES_PSK : CYW43_AUTH_OPEN;
	// Note: `channel` is ignored if `bssid` is not set. Leave it at 0 and see what happens
	const uint8_t* bssid = cfg.bssid ? &cfg.bssid[0] : nullptr;
	unsigned channel{0};
	int res = cyw43_wifi_join(&cyw43_state, cfg.ssid.length(), reinterpret_cast<const uint8_t*>(cfg.ssid.c_str()),
							  cfg.password.length(), reinterpret_cast<const uint8_t*>(cfg.password.c_str()), auth,
							  bssid, channel);
	return res == 0;
}

bool StationImpl::disconnect()
{
	int res = cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
	return res == 0;
}

bool StationImpl::isEnabledDHCP() const
{
	return cyw43_state.dhcp_client.state != DHCP_STATE_OFF;
}

void StationImpl::enableDHCP(bool enable)
{
	debug_i("%s NOT IMPLEMENTED", __PRETTY_FUNCTION__);
}

void StationImpl::setHostname(const String& hostname)
{
	this->hostname = hostname;
	netif_set_hostname(&cyw43_state.netif[CYW43_ITF_STA], this->hostname.c_str());
}

String StationImpl::getHostname() const
{
	return netif_get_hostname(&cyw43_state.netif[CYW43_ITF_STA]);
}

IpAddress StationImpl::getIP() const
{
	auto netif = &cyw43_state.netif[CYW43_ITF_STA];
	return *netif_ip_addr4(netif);
}

MacAddress StationImpl::getMacAddress() const
{
	MacAddress mac;
	cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, &mac[0]);
	return mac;
}

bool StationImpl::setMacAddress(const MacAddress& addr) const
{
	// `cyw43_poll` gets set by cyw43_ensure_up(), after which point MAC cannot be changed
	if(cyw43_poll != nullptr) {
		debug_e("Set STA MAC failed, already initialised");
		return false;
	}

	addr.getOctets(cyw43_state.mac);
	return true;
}

IpAddress StationImpl::getNetworkBroadcast() const
{
	auto netif = &cyw43_state.netif[CYW43_ITF_STA];
	return netif_ip_addr4(netif)->addr | ~netif_ip_netmask4(netif)->addr;
}

IpAddress StationImpl::getNetworkMask() const
{
	auto netif = &cyw43_state.netif[CYW43_ITF_STA];
	return *netif_ip_netmask4(netif);
}

IpAddress StationImpl::getNetworkGateway() const
{
	auto netif = &cyw43_state.netif[CYW43_ITF_STA];
	return *netif_ip_gw4(netif);
}

bool StationImpl::setIP(IpAddress address, IpAddress netmask, IpAddress gateway)
{
	auto netif = &cyw43_state.netif[CYW43_ITF_STA];
	netif_set_ipaddr(netif, address);
	netif_set_netmask(netif, netmask);
	netif_set_gw(netif, gateway);
	return true;
}

String StationImpl::getSSID() const
{
	return cfg.ssid;
}

MacAddress StationImpl::getBSSID() const
{
	return whd_get_bssid(CYW43_ITF_STA);
}

int8_t StationImpl::getRssi() const
{
	int32_t rssi{0};
	int res = cyw43_wifi_get_rssi(&cyw43_state, &rssi);
	return (res == 0) ? rssi : 0;
}

uint8_t StationImpl::getChannel() const
{
	return whd_get_channel(CYW43_ITF_STA);
}

String StationImpl::getPassword() const
{
	return cfg.password;
}

StationConnectionStatus StationImpl::getConnectionStatus() const
{
	int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
	switch(link_status) {
	case CYW43_LINK_JOIN:
	case CYW43_LINK_NOIP:
		return eSCS_Connecting;
	case CYW43_LINK_UP:
		return eSCS_GotIP;
	case CYW43_LINK_FAIL:
		return eSCS_ConnectionFailed;
	case CYW43_LINK_NONET:
		return eSCS_AccessPointNotFound;
	case CYW43_LINK_BADAUTH:
		return eSCS_WrongPassword;
	case CYW43_LINK_DOWN:
	default:
		return enabled ? eSCS_ConnectionFailed : eSCS_Idle;
	}
}

static WifiAuthMode translateAuthMode(uint32_t auth_mode)
{
	// Docs are wrong...
	switch(auth_mode) {
	case 0:
		return AUTH_OPEN; // 0
	case 2:
		return AUTH_WPA_PSK; // 2?
	case 5:
		return AUTH_WPA2_PSK; // 5
	case 7:
		return AUTH_WPA_WPA2_PSK; // 7
	default:
		return AUTH_MAX;
	}
}

void StationImpl::eventHandler(EventInfo& info)
{
	switch(info.ev.event_type) {
	case CYW43_EV_ESCAN_RESULT:
		if(info.self.wifi_scan_state == 1 && info.ev.status == CYW43_STATUS_SUCCESS) {
			station.scanCompleted(true);
		}
		break;

	case CYW43_EV_SET_SSID:
		if(info.ev.status == CYW43_STATUS_SUCCESS) {
			// auto& r = info.ev.u.scan_result;
			// memcpy(ssid, r.ssid, sizeof(ssid));
		}
		break;

	case CYW43_EV_DISASSOC_IND:
	case CYW43_EV_DEAUTH_IND:
		break;
	}
}

void StationImpl::scanCompleted(bool result)
{
	debug_i("scanCompleted(), result %u", result);
	scanTimer.stop();
	if(scanCompletedCallback) {
		scanCompletedCallback(result, scanResults);
		scanCompletedCallback = nullptr;
	}
	scanResults.clear();
}

bool StationImpl::startScan(ScanCompletedDelegate scanCompleted)
{
	if(scanCompletedCallback) {
		debug_e("[STA] Scan in progress");
		return false;
	}

	int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
	switch(link_status) {
	case CYW43_LINK_JOIN:
	case CYW43_LINK_NOIP:
		debug_e("[STA] Invalid WiFi state for scan");
		return false;
	}

	scanResults.clear();
	scanCompletedCallback = scanCompleted;
	if(!scanCompleted) {
		return false;
	}

	auto scan_result = [](void* env, const cyw43_ev_scan_result_t* result) -> int {
		if(!result || !station.scanCompletedCallback) {
			return 0;
		}

		MacAddress bssid{result->bssid};
		for(auto& r : scanResults) {
			if(r.bssid == bssid) {
				return 0; // Already got
			}
		}

		auto r = new BssInfo{
			.bssid = bssid,
			.authorization = translateAuthMode(result->auth_mode),
			.channel = uint8_t(result->channel),
			.rssi = result->rssi,
		};
		if(result->ssid_len == 0 || result->ssid[0] == 0) {
			r->hidden = true;
		} else {
			r->ssid.setString(reinterpret_cast<const char*>(result->ssid), result->ssid_len);
		}
		scanResults.addElement(r);
		return 0;
	};

	cyw43_wifi_scan_options_t scan_options{};
	int err = cyw43_wifi_scan(&cyw43_state, &scan_options, nullptr, scan_result);
	if(err != 0) {
		debug_e("startScan failed %d", err);
		return false;
	}

	scanTimer.initializeMs<SCAN_TIMEOUT_MS>([]() { station.scanCompleted(false); });
	scanTimer.startOnce();

	return true;
}

void StationImpl::onSystemReady()
{
	debug_i("%s", __PRETTY_FUNCTION__);
}

#ifdef ENABLE_SMART_CONFIG

void StationImpl::internalSmartConfig(sc_status status, void* pdata)
{
}

bool StationImpl::smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback)
{
	return false;
}

void StationImpl::smartConfigStop()
{
}

#endif // ENABLE_SMART_CONFIG

#ifdef ENABLE_WPS

bool StationImpl::wpsConfigure(uint8_t credIndex)
{
	return false;
}

bool StationImpl::wpsConfigStart(WPSConfigDelegate callback)
{
	return false;
}

void StationImpl::wpsConfigStop()
{
}

#endif // ENABLE_WPS

} // namespace SmingInternal::Network
