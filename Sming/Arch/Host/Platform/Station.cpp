/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Station.cpp
 *
 ****/

#include "Platform/Station.h"
#include "Data/HexString.h"

StationClass WifiStation;

void StationClass::enable(bool enabled, bool save)
{
}

bool StationClass::isEnabled()
{
	return true;
}

bool StationClass::config(const String& ssid, const String& password, bool autoConnectOnStartup, bool save)
{
	return true;
}

bool StationClass::connect()
{
	return true;
}

bool StationClass::disconnect()
{
	return true;
}

bool StationClass::isConnected()
{
	return true;
}

bool StationClass::isConnectionFailed()
{
	return false;
}

bool StationClass::isEnabledDHCP()
{
	return true;
}

void StationClass::enableDHCP(bool enable)
{
}

void StationClass::setHostname(const String& hostname)
{
}

String StationClass::getHostname()
{
	return nullptr;
}

IPAddress StationClass::getIP()
{
	struct ip_info info = {0};
	wifi_get_ip_info(STATION_IF, &info);
	return info.ip;
}

String StationClass::getMAC(char sep)
{
	uint8 hwaddr[6];
	if(wifi_get_macaddr(STATION_IF, hwaddr)) {
		return makeHexString(hwaddr, sizeof(hwaddr), sep);
	} else {
		return nullptr;
	}
}

IPAddress StationClass::getNetworkBroadcast()
{
	struct ip_info info = {0};
	wifi_get_ip_info(STATION_IF, &info);
	return (info.ip.addr | ~info.netmask.addr);
}

IPAddress StationClass::getNetworkMask()
{
	struct ip_info info = {0};
	wifi_get_ip_info(STATION_IF, &info);
	return info.netmask;
}

IPAddress StationClass::getNetworkGateway()
{
	struct ip_info info = {0};
	wifi_get_ip_info(STATION_IF, &info);
	return info.gw;
}

bool StationClass::setIP(IPAddress address)
{
	return false;
}

bool StationClass::setIP(IPAddress address, IPAddress netmask, IPAddress gateway)
{
	return false;
}

String StationClass::getSSID()
{
	return nullptr;
}

int8_t StationClass::getRssi()
{
	return -120;
}

uint8_t StationClass::getChannel()
{
	return 0;
}

String StationClass::getPassword()
{
	return nullptr;
}

EStationConnectionStatus StationClass::getConnectionStatus()
{
	return eSCS_GotIP;
}

bool StationClass::startScan(ScanCompletedDelegate scanCompleted)
{
	scanCompletedCallback = scanCompleted;
	if(!scanCompleted) {
		return false;
	}

	host_queue_callback(
		[](uint32_t param) {
			auto self = reinterpret_cast<StationClass*>(param);
			BssList list;
			BssInfo info(nullptr);
			info.ssid = "Dummy SSID";
			info.channel = 1;
			info.rssi = -50;
			info.hidden = false;
			info.authorization = AUTH_OPEN;
			wifi_get_macaddr(STATION_IF, info.bssid);
			list.add(info);
			self->scanCompletedCallback(true, list);
		},
		uint32_t(this));

	return true;
}

void StationClass::onSystemReady()
{
}

const char* StationClass::getConnectionStatusName()
{
	switch(getConnectionStatus()) {
	case eSCS_Idle:
		return "Idle";
	case eSCS_Connecting:
		return "Connecting";
	case eSCS_WrongPassword:
		return "Wrong password";
	case eSCS_AccessPointNotFound:
		return "Access point not found";
	case eSCS_ConnectionFailed:
		return "Connection failed";
	case eSCS_GotIP:
		return "Successful connected";
	default:
		SYSTEM_ERROR("Unknown status: %d", getConnectionStatus());
		return "";
	};
}

void StationClass::smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback)
{
}

void StationClass::smartConfigStop()
{
}

#ifdef ENABLE_WPS
void StationClass::internalWpsConfig(wps_cb_status status)
{
}

bool StationClass::wpsConfigStart(WPSConfigDelegate callback)
{
	return false;
}

bool StationClass::beginWPSConfig()
{
	debugf("StationClass::beginWPSConfig()\n");
	return (wpsConfigStart());
}

void StationClass::wpsConfigStop()
{
}
#endif

////////////

BssInfo::BssInfo(bss_info* info)
{
}

const char* BssInfo::getAuthorizationMethodName()
{
	switch(authorization) {
	case AUTH_OPEN:
		return "OPEN";
	case AUTH_WEP:
		return "WEP";
	case AUTH_WPA_PSK:
		return "WPA_PSK";
	case AUTH_WPA2_PSK:
		return "WPA2_PSK";
	case AUTH_WPA_WPA2_PSK:
		return "WPA_WPA2_PSK";
	default:
		SYSTEM_ERROR("Unknown auth: %d", authorization);
		return "";
	}
}

uint32_t BssInfo::getHashId()
{
	uint32_t a = bssid[4] | (bssid[5] << 8);
	uint32_t b = bssid[0] | (bssid[1] << 8) | (bssid[2] << 16) | (bssid[3] << 24);
	return a ^ b;
}
