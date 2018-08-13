/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/*
 * 14/8/2018 (mikee47)
 *
 * 	In config() method if we failed to read the existing config it got a debug message
 * 	but otherwise carried on as if nothing had happened. Instead, this forces the
 * 	requested configuration change as a write may still succeed.
 *	The function has also been modified to simplify. sta_config_equal() added.
 */

#include "Station.h"
#include "Interrupts.h"

StationClass WifiStation;

void StationClass::enable(bool enabled, bool save)
{
	uint8 mode;
	if (save)
		mode = wifi_get_opmode_default() & ~STATION_MODE;
	else
		mode = wifi_get_opmode() & ~STATION_MODE;
	if (enabled)
		mode |= STATION_MODE;
	if (save)
		wifi_set_opmode(mode);
	else
		wifi_set_opmode_current(mode);
}

/** @brief compare two STA configurations
 * 	@param lhs station_config
 * 	@param rhs station_config
 * 	@retval bool true if configs are the same
 *	@note taken from the ESP8266 Arduino source, but modified to use safe string compare
 */
static bool sta_config_equal(const station_config& lhs, const station_config& rhs)
{
	if (strncmp((const char*)lhs.ssid, (const char*)rhs.ssid, sizeof(lhs.ssid)))
		return false;

	//in case of password, use strncmp with size 64 to cover 64byte psk case (no null term)
	if (strncmp((const char*)lhs.password, (const char*)rhs.password, sizeof(lhs.password)))
		return false;

	if (lhs.bssid_set != rhs.bssid_set)
		return false;

	if (lhs.bssid_set && memcmp(lhs.bssid, rhs.bssid, sizeof(lhs.bssid)))
		return false;

	return true;
}

bool StationClass::config(const String& ssid, const String& password, bool autoConnectOnStartup, bool save)
{
	station_config config = {0};

	if (ssid.length() >= sizeof(config.ssid)) {
		debug_e("Station SSID too long");
		return false;
	}
	if (password.length() >= sizeof(config.password)) {
		debug_e("Station Password too long");
		return false;
	}

	bool enabled = isEnabled();
	bool dhcp = isEnabledDHCP();

	// Power on for configuration
	if (!enabled)
		enable(true);

	bool cfgValid = wifi_station_get_config(&config);
	if (!cfgValid)
		debugf("Can't read station configuration!");

	station_config newConfig = {0};
	newConfig.bssid_set = false;
	ssid.getBytes(newConfig.ssid, sizeof(newConfig.ssid));
	password.getBytes(newConfig.password, sizeof(newConfig.password));

	bool success;

	if (cfgValid && sta_config_equal(config, newConfig)) {
		debugf("Station configuration is: %s", ssid.c_str());
		success = true;
	}
	else {
		noInterrupts();

		if (save)
			success = wifi_station_set_config(&newConfig);
		else
			success = wifi_station_set_config_current(&newConfig);

		interrupts();
	}

	if (success)
		debugf("Station configuration was updated to: %s", ssid.c_str());
	else
		debugf("Can't set station configuration!");

	if (!dhcp)
		enableDHCP(dhcp);
	if (!enabled)
		enable(enabled);

	if (success)
		wifi_station_set_auto_connect(autoConnectOnStartup);

	return success;
}

bool StationClass::isConnected()
{
	if (getConnectionStatus() != eSCS_GotIP)
		return false;
	if (getIP().isNull())
		return false;

	return true;
}

bool StationClass::isConnectionFailed()
{
	EStationConnectionStatus status = getConnectionStatus();
	return status == eSCS_WrongPassword || status == eSCS_AccessPointNotFound || status == eSCS_ConnectionFailed;
}

IPAddress StationClass::getIP()
{
	struct ip_info info = {0};
	wifi_get_ip_info(STATION_IF, &info);
	return info.ip;
}

String StationClass::getMAC()
{
	uint8 hwaddr[6];
	if (wifi_get_macaddr(STATION_IF, hwaddr))
		return toHexString(hwaddr, sizeof(hwaddr), ':');
	else
		return nullptr;
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
	IPAddress mask = IPAddress(255, 255, 255, 0);
	IPAddress gateway = IPAddress(address);
	gateway[3] = 1; // x.x.x.1
	return setIP(address, mask, gateway);
}

bool StationClass::setIP(IPAddress address, IPAddress netmask, IPAddress gateway)
{
	if (System.isReady()) {
		debugf("IP can be changed only in init() method");
		return false;
	}

	wifi_station_disconnect();
	wifi_station_dhcpc_stop();
	struct ip_info ipinfo;
	wifi_get_ip_info(STATION_IF, &ipinfo);
	ipinfo.ip = address;
	ipinfo.netmask = netmask;
	ipinfo.gw = gateway;
	if (wifi_set_ip_info(STATION_IF, &ipinfo))
		debugf("Station IP succesfully updated");
	else {
		debugf("Station IP can't be updated");
		enableDHCP(true);
	}
	wifi_station_connect();
	//wifi_station_dhcpc_start();
	return true;
}

String StationClass::getSSID()
{
	station_config config = {0};
	if (!wifi_station_get_config(&config)) {
		debugf("Can't read station configuration!");
		return nullptr;
	}
	debugf("SSID: %s", (char*)config.ssid);
	return String((char*)config.ssid);
}

sint8 StationClass::getRssi()
{
	auto rssi = wifi_station_get_rssi();
	debugf("Rssi: %d dBm", rssi);
	return rssi;
}

uint8 StationClass::getChannel()
{
	auto channel = wifi_get_channel();
	debugf("Channel: %d CH", channel);
	return channel;
}

String StationClass::getPassword()
{
	station_config config = {0};
	if (!wifi_station_get_config(&config)) {
		debugf("Can't read station configuration!");
		return nullptr;
	}

	debugf("Pass: %s", (char*)config.password);
	return String((char*)config.password);
}

bool StationClass::startScan(ScanCompletedDelegate scanCompleted)
{
	scanCompletedCallback = scanCompleted;
	if (!scanCompleted)
		return false;

	bool res = wifi_station_scan(nullptr, staticScanCompleted);
	if (!res) {
		if (!System.isReady()) {
			// It's OK, queue this task
			_runScan = true;
			return true;
		}
		debugf("startScan failed");
	}
	return res;
}

////////////

void StationClass::staticScanCompleted(void* arg, STATUS status)
{
	BssList list;

	if (status == OK) {
		auto cur = (bss_info*)arg;
		while (cur) {
			list.add(BssInfo(cur));
			cur = cur->next.stqe_next;
		}
	}

	if (WifiStation.scanCompletedCallback)
		WifiStation.scanCompletedCallback(status == OK, list);

	if (status == OK)
		debugf("scan completed: %d found", list.count());
	else
		debugf("scan failed %d", status);
}

void StationClass::onSystemReady()
{
	if (_runScan) {
		wifi_station_scan(nullptr, staticScanCompleted);
		_runScan = false;
	}
}

String StationClass::getConnectionStatusName()
{
	switch (getConnectionStatus()) {
	case eSCS_Idle:
		return F("Idle");
	case eSCS_Connecting:
		return F("Connecting");
	case eSCS_WrongPassword:
		return F("Wrong password");
	case eSCS_AccessPointNotFound:
		return F("Access point not found");
	case eSCS_ConnectionFailed:
		return F("Connection failed");
	case eSCS_GotIP:
		return F("Successful connected");
	default:
		SYSTEM_ERROR("Unknown status: %d", getConnectionStatus());
		return nullptr;
	};
}

void StationClass::staticSmartConfigCallback(sc_status status, void* pdata)
{
	WifiStation.internalSmartConfig(status, pdata);
}

void StationClass::internalSmartConfig(sc_status status, void* pdata)
{
	if (smartConfigCallback) {
		smartConfigCallback(status, pdata);
		return;
	}

	switch (status) {
	case SC_STATUS_WAIT:
		debugf("SC_STATUS_WAIT\n");
		break;

	case SC_STATUS_FIND_CHANNEL:
		debugf("SC_STATUS_FIND_CHANNEL\n");
		break;

	case SC_STATUS_GETTING_SSID_PSWD:
		debugf("SC_STATUS_GETTING_SSID_PSWD\n");
		break;

	case SC_STATUS_LINK: {
		debugf("SC_STATUS_LINK\n");
		station_config* sta_conf = (station_config*)pdata;
		char* ssid = (char*)sta_conf->ssid;
		char* password = (char*)sta_conf->password;
		config(ssid, password);
		connect();
		break;
	}

	case SC_STATUS_LINK_OVER:
		debugf("SC_STATUS_LINK_OVER\n");
		smartConfigStop();
		break;
	}
}

void StationClass::smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback)
{
	smartConfigCallback = callback;
	smartconfig_set_type((sc_type)sctype);
	smartconfig_start(staticSmartConfigCallback);
}

void StationClass::smartConfigStop()
{
	smartconfig_stop();
	smartConfigCallback = nullptr;
}

#ifdef ENABLE_WPS

void StationClass::internalWpsConfig(wps_cb_status status)
{
	bool processInternal = true;
	if (wpsConfigCallback)
		processInternal = wpsConfigCallback(status);

	if (processInternal) {
		switch (status) {
		case WPS_CB_ST_SUCCESS:
			debugf("wifi_wps_status_cb(): WPS_CB_ST_SUCCESS\n");
			wpsConfigStop();
			connect();
			break;

		case WPS_CB_ST_FAILED:
			debugf("wifi_wps_status_cb(): WPS_CB_ST_FAILED\n");
			wpsConfigStop();
			connect(); // try to reconnect with old config
			break;

		case WPS_CB_ST_TIMEOUT:
			debugf("wifi_wps_status_cb(): WPS_CB_ST_TIMEOUT\n");
			wpsConfigStop();
			connect(); // try to reconnect with old config
			break;

		case WPS_CB_ST_WEP:
			debugf("wifi_wps_status_cb(): WPS_CB_ST_WEP\n");
			break;

		default:
			debugf("wifi_wps_status_cb(): unknown wps_cb_status %d\n", status);
			wpsConfigStop();
			connect(); // try to reconnect with old config
		}
	}
}

bool StationClass::wpsConfigStart(WPSConfigDelegate callback)
{
	debugf("WPS start\n");
	wpsConfigCallback = callback;
	wifi_station_disconnect();
	wifi_set_opmode_current(wifi_get_opmode() | STATION_MODE);
	debugf("WPS stationmode activated\n");

	if (!wifi_wps_enable(WPS_TYPE_PBC)) {
		debugf("StationClass::wpsConfigStart() : wps enable failed\n");
		return false;
	}

	if (!wifi_set_wps_cb((wps_st_cb_t)&staticWpsConfigCallback)) {
		debugf("StationClass::wpsConfigStart() : cb failed\n");
		return false;
	}

	if (!wifi_wps_start()) {
		debugf("StationClass::wpsConfigStart() : wifi_wps_start() failed\n");
		return false;
	}

	return true;
}

bool StationClass::beginWPSConfig()
{
	debugf("StationClass::beginWPSConfig()\n");
	return wpsConfigStart();
}

void StationClass::wpsConfigStop()
{
	if (!wifi_wps_disable())
		debugf("StationClass::wpsConfigStop() : wifi_wps_disable() failed\n");
}

#endif

/* BssInfo */

BssInfo::BssInfo(bss_info* info)
{
	ssid = String((char*)info->ssid);
	memcpy(bssid, info->bssid, sizeof(bssid));
	authorization = info->authmode;
	channel = info->channel;
	rssi = info->rssi;
	hidden = info->is_hidden;
}

String BssInfo::getAuthorizationMethodName()
{
	switch (authorization) {
	case AUTH_OPEN:
		return F("OPEN");
	case AUTH_WEP:
		return F("WEP");
	case AUTH_WPA_PSK:
		return F("WPA_PSK");
	case AUTH_WPA2_PSK:
		return F("WPA2_PSK");
	case AUTH_WPA_WPA2_PSK:
		return F("WPA_WPA2_PSK");
	default:
		SYSTEM_ERROR("Unknown auth: %d", authorization);
		return nullptr;
	}
}

uint32_t BssInfo::getHashId()
{
	uint32_t a = *(uint16_t*)(&bssid[4]);
	uint32_t b = *(uint32_t*)bssid;
	return a ^ b;
}
