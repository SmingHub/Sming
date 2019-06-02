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
#include "Interrupts.h"
#include "Data/HexString.h"

StationClass WifiStation;

void StationClass::enable(bool enabled, bool save)
{
	uint8 mode;
	if(save)
		mode = wifi_get_opmode_default() & ~STATION_MODE;
	else
		mode = wifi_get_opmode() & ~STATION_MODE;
	if(enabled)
		mode |= STATION_MODE;
	if(save)
		wifi_set_opmode(mode);
	else
		wifi_set_opmode_current(mode);
}

bool StationClass::isEnabled()
{
	return wifi_get_opmode() & STATION_MODE;
}

bool StationClass::config(const String& ssid, const String& password, bool autoConnectOnStartup, bool save)
{
	station_config config = {0};

	if(ssid.length() >= sizeof(config.ssid))
		return false;
	if(password.length() >= sizeof(config.password))
		return false;

	bool enabled = isEnabled();
	bool dhcp = isEnabledDHCP();
	if(!enabled)
		enable(true); // Power on for configuration

	bool cfgreaded = wifi_station_get_config(&config);
	if(!cfgreaded)
		debugf("Can't read station configuration!");

	if(strncmp(ssid.c_str(), (char*)config.ssid, sizeof(config.ssid)) != 0 ||
	   strncmp(password.c_str(), (char*)config.password, sizeof(config.password)) != 0 || config.bssid_set) {
		memset(config.ssid, 0, sizeof(config.ssid));
		memset(config.password, 0, sizeof(config.password));
		config.bssid_set = false;
		strcpy((char*)config.ssid, ssid.c_str());
		strcpy((char*)config.password, password.c_str());

		noInterrupts();

		bool success = false;
		if(save) {
			success = wifi_station_set_config(&config);
		} else {
			success = wifi_station_set_config_current(&config);
		}

		if(!success) {
			interrupts();
			debugf("Can't set station configuration!");
			if(!dhcp)
				enableDHCP(dhcp);
			if(!enabled)
				enable(enabled);
			return false;
		}
		debugf("Station configuration was updated to: %s", ssid.c_str());

		interrupts();
	} else
		debugf("Station configuration is: %s", ssid.c_str());
	if(!dhcp)
		enableDHCP(dhcp);
	if(!enabled)
		enable(enabled);

	wifi_station_set_auto_connect(autoConnectOnStartup);

	return true;
}

bool StationClass::connect()
{
	return wifi_station_connect();
}

bool StationClass::disconnect()
{
	return wifi_station_disconnect();
}

bool StationClass::isConnected()
{
	if(getConnectionStatus() != eSCS_GotIP)
		return false;
	if(getIP().isNull())
		return false;

	return true;
}

bool StationClass::isConnectionFailed()
{
	EStationConnectionStatus status = getConnectionStatus();
	return status == eSCS_WrongPassword || status == eSCS_AccessPointNotFound || status == eSCS_ConnectionFailed;
}

bool StationClass::isEnabledDHCP()
{
	return wifi_station_dhcpc_status() == DHCP_STARTED;
}

void StationClass::enableDHCP(bool enable)
{
	if(enable)
		wifi_station_dhcpc_start();
	else
		wifi_station_dhcpc_stop();
}

void StationClass::setHostname(const String& hostname)
{
	wifi_station_set_hostname((char*)hostname.c_str());
}

String StationClass::getHostname()
{
	return (String)wifi_station_get_hostname();
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
	if(wifi_get_macaddr(STATION_IF, hwaddr))
		return makeHexString(hwaddr, sizeof(hwaddr), sep);
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
	if(System.isReady()) {
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
	if(wifi_set_ip_info(STATION_IF, &ipinfo))
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
	if(!wifi_station_get_config(&config)) {
		debugf("Can't read station configuration!");
		return "";
	}
	debugf("SSID: %s", (char*)config.ssid);
	return String((char*)config.ssid);
}

int8_t StationClass::getRssi()
{
	debugf("Rssi: %d dBm", wifi_station_get_rssi());
	return wifi_station_get_rssi();
}

uint8_t StationClass::getChannel()
{
	debugf("Channel: %d CH", wifi_get_channel());
	return wifi_get_channel();
}

String StationClass::getPassword()
{
	station_config config = {0};
	if(!wifi_station_get_config(&config)) {
		debugf("Can't read station configuration!");
		return "";
	}
	debugf("Pass: %s", (char*)config.password);
	return String((char*)config.password);
}

EStationConnectionStatus StationClass::getConnectionStatus()
{
	return (EStationConnectionStatus)wifi_station_get_connect_status();
}

bool StationClass::startScan(ScanCompletedDelegate scanCompleted)
{
	scanCompletedCallback = scanCompleted;
	if(!scanCompleted)
		return false;

	bool res = wifi_station_scan(nullptr, staticScanCompleted);
	if(!res) {
		if(!System.isReady()) {
			// It's OK, queue this task
			runScan = true;
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
	if(status == OK) {
		if(WifiStation.scanCompletedCallback) {
			bss_info* cur = (bss_info*)arg;

			while(cur != nullptr) {
				list.add(BssInfo(cur));
				cur = cur->next.stqe_next;
			}
			WifiStation.scanCompletedCallback(true, list);
		}

		debugf("scan completed: %d found", list.count());
	} else {
		debugf("scan failed %d", status);
		if(WifiStation.scanCompletedCallback)
			WifiStation.scanCompletedCallback(false, list);
	}
}

void StationClass::onSystemReady()
{
	if(runScan) {
		wifi_station_scan(nullptr, staticScanCompleted);
		runScan = false;
	}
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

void StationClass::staticSmartConfigCallback(sc_status status, void* pdata)
{
	WifiStation.internalSmartConfig(status, pdata);
}

void StationClass::internalSmartConfig(sc_status status, void* pdata)
{
	if(smartConfigCallback) {
		smartConfigCallback(status, pdata);
		return;
	}

	switch(status) {
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
	} break;
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
	if(wpsConfigCallback) {
		processInternal = wpsConfigCallback(status);
	}
	if(processInternal) {
		switch(status) {
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

void StationClass::staticWpsConfigCallback(wps_cb_status status)
{
	WifiStation.internalWpsConfig(status);
}

bool StationClass::wpsConfigStart(WPSConfigDelegate callback)
{
	debugf("WPS start\n");
	wpsConfigCallback = callback;
	wifi_station_disconnect();
	wifi_set_opmode_current(wifi_get_opmode() | STATION_MODE);
	debugf("WPS stationmode activated\n");
	if(!wifi_wps_enable(WPS_TYPE_PBC)) {
		debugf("StationClass::wpsConfigStart() : wps enable failed\n");
		return (false);
	}
	if(!wifi_set_wps_cb((wps_st_cb_t)&staticWpsConfigCallback)) {
		debugf("StationClass::wpsConfigStart() : cb failed\n");
		return (false);
	}

	if(!wifi_wps_start()) {
		debugf("StationClass::wpsConfigStart() : wifi_wps_start() failed\n");
		return (false);
	}
	return (true);
}

bool StationClass::beginWPSConfig()
{
	debugf("StationClass::beginWPSConfig()\n");
	return (wpsConfigStart());
}

void StationClass::wpsConfigStop()
{
	if(!wifi_wps_disable()) {
		debugf("StationClass::wpsConfigStop() : wifi_wps_disable() failed\n");
	}
}
#endif

////////////

BssInfo::BssInfo(bss_info* info)
{
	ssid = reinterpret_cast<const char*>(info->ssid);
	memcpy(bssid, info->bssid, sizeof(bssid));
	authorization = info->authmode;
	channel = info->channel;
	rssi = info->rssi;
	hidden = info->is_hidden;
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
