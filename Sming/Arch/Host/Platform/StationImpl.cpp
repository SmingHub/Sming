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
#include <lwip/netif.h>
#include <lwip/dhcp.h>

StationImpl station;
StationClass& WifiStation = station;

static StationImpl::ApInfo apInfoList[] = {
	{
		.ssid = "Host WiFi",
		.bssid = {},
		.authMode = AUTH_OPEN,
		.channel = 1,
		.hidden = false,
	},
	{
		.ssid = "Private WiFi",
		.bssid = {},
		.authMode = AUTH_WPA2_PSK,
		.channel = 8,
		.hidden = false,
		.pwd = "pass",
	},
	{
		.ssid = "George's Cafe",
		.bssid = {},
		.authMode = AUTH_OPEN,
		.channel = 5,
		.hidden = false,
	},
	{
		.ssid = "Dentist",
		.bssid = {},
		.authMode = AUTH_WPA_WPA2_PSK,
		.channel = 11,
		.hidden = false,
		.pwd = "fillings",
	},
	{
		.ssid = "Can you see me",
		.bssid = {},
		.authMode = AUTH_WPA_PSK,
		.channel = 6,
		.hidden = true,
		.pwd = "secret",
	},
};

// Called directly from startup code
void host_wifi_lwip_init_complete(void)
{
	station.initialise(netif_default);
}

static int getRandomRssi()
{
	return int(os_random() % 50) - 90;
}

class BssInfoImpl : public BssInfo
{
public:
	explicit BssInfoImpl(const StationImpl::ApInfo& info)
	{
		ssid = info.ssid;
		authorization = info.authMode;
		channel = info.channel;
		rssi = getRandomRssi();
		hidden = info.hidden;
	}
};

StationImpl::StationImpl() : currentAp(&apInfoList[0]), savedAp(&apInfoList[0])
{
}

void StationImpl::initialise(netif* nif)
{
	if(nif == nullptr) {
		connectionStatus = eSCS_AccessPointNotFound;
		return;
	}

	auto netif_callback = [](netif* nif) {
		host_queue_callback([](uint32_t param) { station.statusCallback(reinterpret_cast<netif*>(param)); },
							uint32_t(nif));
	};

	netif_set_status_callback(nif, netif_callback);

	netif_set_up(nif);
#if LWIP_IPV6
	netif_create_ip6_linklocal_address(&nif, 1);
#endif

	connectionStatus = eSCS_Connecting;
	if(ip4_addr_isany_val(nif->ip_addr)) {
		dhcp_start(nif);
	}
}

void StationImpl::statusCallback(netif* nif)
{
	if(nif == nullptr) {
		return;
	}

	static uint32_t prev_flags;

	uint32_t changed_flags = prev_flags ^ nif->flags;
	prev_flags = nif->flags;

	if(changed_flags & NETIF_FLAG_UP) {
		assert(currentAp != nullptr);
		if(nif->flags & NETIF_FLAG_UP) {
			host_printf("IF_UP, AP: %s\n", currentAp->ssid);
			wifiEventsImpl.stationConnected(*currentAp);
		} else {
			host_printf("IF_DOWN, AP: %s\n", currentAp->ssid);
			wifiEventsImpl.stationDisconnected(*currentAp, WIFI_DISCONNECT_REASON_CONNECTION_FAIL);
		}
	}

	if((nif->flags & NETIF_FLAG_UP) && !ip4_addr_isany_val(nif->ip_addr)) {
		connectionStatus = eSCS_GotIP;
		if(ipaddr != nif->ip_addr || netmask != nif->netmask || gateway != nif->gw) {
			ipaddr = nif->ip_addr;
			netmask = nif->netmask;
			gateway = nif->gw;
			host_printf("IP_CHANGE, ip: %s, netmask: %s, gateway: %s\n", ipaddr.toString().c_str(),
						netmask.toString().c_str(), gateway.toString().c_str());
			wifiEventsImpl.stationGotIp(ipaddr, netmask, gateway);
		}
	} else {
		hostmsg("No IP address");
		connectionStatus = eSCS_ConnectionFailed;
	}
}

void StationImpl::enable(bool enabled, bool save)
{
	if(save) {
		savedConfig.enabled = enabled;
	} else {
		currentConfig.enabled = enabled;
	}
}

bool StationImpl::isEnabled() const
{
	return currentConfig.enabled;
}

bool StationImpl::config(const String& ssid, const String& password, bool autoConnectOnStartup, bool save)
{
	for(auto& ap : apInfoList) {
		if(ssid == ap.ssid) {
			if(ap.authMode != AUTH_OPEN) {
				if(password != ap.pwd) {
					debug_w("Bad password for '%s'", ssid.c_str());
					return false;
				}
			}

			currentAp = &ap;
			if(save) {
				savedAp = &ap;
			}

			debug_i("Connected to SSID '%s'", ssid.c_str());

			autoConnect = autoConnectOnStartup;
			return true;
		}
	}

	debug_w("SSID '%s' not found", ssid.c_str());
	return false;
}

bool StationImpl::connect()
{
	netif* nif = netif_default;

	if(nif == nullptr) {
		return false;
	}

	netif_set_link_up(nif);
	return true;
}

bool StationImpl::disconnect()
{
	netif* nif = netif_default;

	if(nif == nullptr) {
		return false;
	}

	netif_set_link_down(nif);
	return true;
}

bool StationImpl::isEnabledDHCP() const
{
	return dhcpEnabled;
}

void StationImpl::enableDHCP(bool enable)
{
	netif* nif = netif_default;
	if(nif != nullptr) {
		if(enable && !dhcpEnabled) {
			dhcp_start(nif);
		} else if(!enable && dhcpEnabled) {
			dhcp_stop(nif);
		}
	}
	dhcpEnabled = enable;
}

void StationImpl::setHostname(const String& hostname)
{
	this->hostName = hostname;
}

String StationImpl::getHostname() const
{
	return hostName;
}

IpAddress StationImpl::getIP() const
{
	return ipaddr;
}

MacAddress StationImpl::getMacAddress() const
{
	netif* nif = netif_default;

	if(nif == nullptr) {
		return MACADDR_NONE;
	} else {
		return MacAddress(nif->hwaddr);
	}
}

IpAddress StationImpl::getNetworkBroadcast() const
{
	return ipaddr | ~netmask;
}

IpAddress StationImpl::getNetworkMask() const
{
	return netmask;
}

IpAddress StationImpl::getNetworkGateway() const
{
	return gateway;
}

bool StationImpl::setIP(IpAddress address, IpAddress netmask, IpAddress gateway)
{
	netif* nif = netif_default;

	if(nif == nullptr) {
		return false;
	}

	netif_set_ipaddr(nif, address);
	netif_set_netmask(nif, netmask);
	netif_set_gw(nif, gateway);
	return true;
}

String StationImpl::getSSID() const
{
	return currentAp ? currentAp->ssid : nullptr;
}

int8_t StationImpl::getRssi() const
{
	return getRandomRssi();
}

uint8_t StationImpl::getChannel() const
{
	return currentAp ? currentAp->channel : 0;
}

String StationImpl::getPassword() const
{
	return currentAp ? currentAp->pwd : nullptr;
}

StationConnectionStatus StationImpl::getConnectionStatus() const
{
	return connectionStatus;
}

bool StationImpl::startScan(ScanCompletedDelegate scanCompleted)
{
	scanCompletedCallback = scanCompleted;
	if(!scanCompleted) {
		return false;
	}

	host_queue_callback(
		[](uint32_t param) {
			auto self = reinterpret_cast<StationImpl*>(param);
			BssList list;
			for(const auto& info : apInfoList) {
				list.addElement(new BssInfoImpl(info));
			}
			self->scanCompletedCallback(true, list);
		},
		uint32_t(this));

	return true;
}

#ifdef ENABLE_SMART_CONFIG

bool StationImpl::smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback)
{
	debug_e("SmartConfig not supported");
	return false;
}

void StationImpl::smartConfigStop()
{
}

#endif // ENABLE_SMART_CONFIG

#ifdef ENABLE_WPS

bool StationImpl::wpsConfigStart(WPSConfigDelegate callback)
{
	debug_e("WPS not supported");
	return false;
}

void StationImpl::wpsConfigStop()
{
}

#endif // ENABLE_WPS
