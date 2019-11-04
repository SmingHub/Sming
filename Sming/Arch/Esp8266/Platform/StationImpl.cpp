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
#include <Interrupts.h>

static StationImpl station;
StationClass& WifiStation = station;

class BssInfoImpl : public BssInfo
{
public:
	explicit BssInfoImpl(const bss_info* info)
	{
		ssid = reinterpret_cast<const char*>(info->ssid);
		bssid = info->bssid;
		authorization = info->authmode;
		channel = info->channel;
		rssi = info->rssi;
		hidden = info->is_hidden;
	}
};

void StationImpl::enable(bool enabled, bool save)
{
	uint8 mode;
	if(save) {
		mode = wifi_get_opmode_default() & ~STATION_MODE;
	} else {
		mode = wifi_get_opmode() & ~STATION_MODE;
	}
	if(enabled) {
		mode |= STATION_MODE;
	}
	if(save) {
		wifi_set_opmode(mode);
	} else {
		wifi_set_opmode_current(mode);
	}
}

bool StationImpl::isEnabled() const
{
	return wifi_get_opmode() & STATION_MODE;
}

bool StationImpl::config(const String& ssid, const String& password, bool autoConnectOnStartup, bool save)
{
	station_config config = {0};
	if(ssid.length() >= sizeof(config.ssid)) {
		return false;
	}
	if(password.length() >= sizeof(config.password)) {
		return false;
	}

	bool enabled = isEnabled();
	bool dhcp = isEnabledDHCP();
	if(!enabled) {
		enable(true, false); // Power on for configuration
	}

	bool setConfig;
	if(wifi_station_get_config(&config)) {
		// Determine if config has changed
		setConfig =
			strncmp(ssid.c_str(), reinterpret_cast<const char*>(config.ssid), sizeof(config.ssid)) != 0 ||
			strncmp(password.c_str(), reinterpret_cast<const char*>(config.password), sizeof(config.password)) != 0 ||
			config.bssid_set;
	} else {
		debugf("Can't read station configuration!");
		setConfig = true;
	}

	bool success;
	if(setConfig) {
		memset(config.ssid, 0, sizeof(config.ssid));
		memset(config.password, 0, sizeof(config.password));
		config.bssid_set = false;
		ssid.getBytes(config.ssid, sizeof(config.ssid));
		password.getBytes(config.password, sizeof(config.password));

		noInterrupts();

		if(save) {
			success = wifi_station_set_config(&config);
		} else {
			success = wifi_station_set_config_current(&config);
		}

		interrupts();

		if(success) {
			debugf("Station configuration was updated to: %s", ssid.c_str());
		} else {
			debugf("Can't set station configuration!");
		}
	} else {
		debugf("Station configuration is: %s", ssid.c_str());
		success = true;
	}

	if(!dhcp) {
		enableDHCP(false);
	}
	if(!enabled) {
		enable(false, false);
	}

	if(success) {
		wifi_station_set_auto_connect(autoConnectOnStartup);
	}

	return success;
}

bool StationImpl::connect()
{
	return wifi_station_connect();
}

bool StationImpl::disconnect()
{
	return wifi_station_disconnect();
}

bool StationImpl::isEnabledDHCP() const
{
	return wifi_station_dhcpc_status() == DHCP_STARTED;
}

void StationImpl::enableDHCP(bool enable)
{
	if(enable) {
		wifi_station_dhcpc_start();
	} else {
		wifi_station_dhcpc_stop();
	}
}

void StationImpl::setHostname(const String& hostname)
{
	wifi_station_set_hostname(const_cast<char*>(hostname.c_str()));
}

String StationImpl::getHostname() const
{
	return wifi_station_get_hostname();
}

IpAddress StationImpl::getIP() const
{
	struct ip_info info = {0};
	wifi_get_ip_info(STATION_IF, &info);
	return info.ip;
}

MacAddress StationImpl::getMacAddress() const
{
	MacAddress addr;
	if(wifi_get_macaddr(STATION_IF, &addr[0])) {
		return addr;
	} else {
		return MACADDR_NONE;
	}
}

IpAddress StationImpl::getNetworkBroadcast() const
{
	struct ip_info info = {0};
	wifi_get_ip_info(STATION_IF, &info);
	return (info.ip.addr | ~info.netmask.addr);
}

IpAddress StationImpl::getNetworkMask() const
{
	struct ip_info info = {0};
	wifi_get_ip_info(STATION_IF, &info);
	return info.netmask;
}

IpAddress StationImpl::getNetworkGateway() const
{
	struct ip_info info = {0};
	wifi_get_ip_info(STATION_IF, &info);
	return info.gw;
}

bool StationImpl::setIP(IpAddress address, IpAddress netmask, IpAddress gateway)
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
	if(wifi_set_ip_info(STATION_IF, &ipinfo)) {
		debugf("Station IP succesfully updated");
	} else {
		debugf("Station IP can't be updated");
		enableDHCP(true);
	}
	wifi_station_connect();
	return true;
}

String StationImpl::getSSID() const
{
	station_config config = {0};
	if(!wifi_station_get_config(&config)) {
		debugf("Can't read station configuration!");
		return "";
	}
	auto ssid = reinterpret_cast<const char*>(config.ssid);
	debugf("SSID: '%s'", ssid);
	return ssid;
}

int8_t StationImpl::getRssi() const
{
	debugf("Rssi: %d dBm", wifi_station_get_rssi());
	return wifi_station_get_rssi();
}

uint8_t StationImpl::getChannel() const
{
	debugf("Channel: %d CH", wifi_get_channel());
	return wifi_get_channel();
}

String StationImpl::getPassword() const
{
	station_config config = {0};
	if(!wifi_station_get_config(&config)) {
		debugf("Can't read station configuration!");
		return nullptr;
	}
	auto pwd = reinterpret_cast<const char*>(config.password);
	debugf("Pass: '%s'", pwd);
	return pwd;
}

StationConnectionStatus StationImpl::getConnectionStatus() const
{
	return StationConnectionStatus(wifi_station_get_connect_status());
}

bool StationImpl::startScan(ScanCompletedDelegate scanCompleted)
{
	scanCompletedCallback = scanCompleted;
	if(!scanCompleted) {
		return false;
	}

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

void StationImpl::staticScanCompleted(void* arg, STATUS status)
{
	BssList list;
	if(status == OK) {
		if(station.scanCompletedCallback) {
			auto cur = static_cast<bss_info*>(arg);
			while(cur != nullptr) {
				list.addElement(new BssInfoImpl(cur));
				cur = cur->next.stqe_next;
			}
			station.scanCompletedCallback(true, list);
		}

		debugf("scan completed: %d found", list.count());
	} else {
		debugf("scan failed %d", status);
		if(station.scanCompletedCallback) {
			station.scanCompletedCallback(false, list);
		}
	}
}

void StationImpl::onSystemReady()
{
	if(runScan) {
		wifi_station_scan(nullptr, staticScanCompleted);
		runScan = false;
	}
}

#ifdef ENABLE_SMART_CONFIG

void StationImpl::internalSmartConfig(sc_status status, void* pdata)
{
	if(smartConfigEventInfo == nullptr) {
		debug_e("smartconfig eventInfo is NULL");
		return;
	}

	auto& evt = *smartConfigEventInfo;

	switch(status) {
	case SC_STATUS_WAIT:
		debugf("SC_STATUS_WAIT\n");
		break;
	case SC_STATUS_FIND_CHANNEL:
		debugf("SC_STATUS_FIND_CHANNEL\n");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		debugf("SC_STATUS_GETTING_SSID_PSWD\n");
		assert(pdata != nullptr);
		smartConfigEventInfo->type = SmartConfigType(*static_cast<sc_type*>(pdata));
		break;
	case SC_STATUS_LINK: {
		debugf("SC_STATUS_LINK\n");
		auto cfg = static_cast<const station_config*>(pdata);
		assert(cfg != nullptr);
		evt.ssid = reinterpret_cast<const char*>(cfg->ssid);
		evt.password = reinterpret_cast<const char*>(cfg->password);
		evt.bssidSet = (cfg->bssid_set != 0);
		evt.bssid = cfg->bssid;
		break;
	}
	case SC_STATUS_LINK_OVER:
		debugf("SC_STATUS_LINK_OVER\n");
		break;
	}

	bool processInternal = true;
	if(smartConfigCallback) {
		processInternal = smartConfigCallback(SmartConfigEvent(status), evt);
	}

	if(processInternal) {
		switch(status) {
		case SC_STATUS_WAIT:
			break;
		case SC_STATUS_FIND_CHANNEL:
			break;
		case SC_STATUS_GETTING_SSID_PSWD:
			break;
		case SC_STATUS_LINK:
			config(evt.ssid, evt.password, true, true);
			connect();
			break;
		case SC_STATUS_LINK_OVER:
			smartConfigStop();
			break;
		}
	}
}

bool StationImpl::smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback)
{
	if(smartConfigEventInfo != nullptr) {
		return false; // Already in progress
	}

	if(!smartconfig_set_type(sc_type(sctype))) {
		debug_e("smartconfig_set_type(%u) failed", sctype);
		return false;
	}

	smartConfigEventInfo = new SmartConfigEventInfo;
	if(smartConfigEventInfo == nullptr) {
		return false;
	}

	// Bug in SDK Version 3 where a debug statement attempts to read from flash and throws a memory exception
	// This is a workaround
	auto os_print = system_get_os_print();
	if(os_print) {
		system_set_os_print(false);
	}

	smartConfigCallback = callback;
	if(!smartconfig_start([](sc_status status, void* pdata) { station.internalSmartConfig(status, pdata); })) {
		debug_e("smartconfig_start() failed");
		smartConfigCallback = nullptr;
		delete smartConfigEventInfo;
		smartConfigEventInfo = nullptr;
		return false;
	}

	if(os_print) {
		system_set_os_print(true);
	}

	return true;
}

void StationImpl::smartConfigStop()
{
	smartconfig_stop();
	smartConfigCallback = nullptr;
	delete smartConfigEventInfo;
	smartConfigEventInfo = nullptr;
}

#endif // ENABLE_SMART_CONFIG

#ifdef ENABLE_WPS

void StationImpl::internalWpsConfig(wps_cb_status status)
{
	bool processInternal = true;
	if(wpsConfigCallback) {
		processInternal = wpsConfigCallback(WpsStatus(status));
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

bool StationImpl::wpsConfigStart(WPSConfigDelegate callback)
{
	debugf("WPS start\n");
	wpsConfigCallback = callback;
	wifi_station_disconnect();
	wifi_set_opmode_current(wifi_get_opmode() | STATION_MODE);
	debugf("WPS stationmode activated\n");
	if(!wifi_wps_enable(WPS_TYPE_PBC)) {
		debugf("Station::wpsConfigStart() : wps enable failed\n");
		return false;
	}
	if(!wifi_set_wps_cb([](int status) { station.internalWpsConfig(wps_cb_status(status)); })) {
		debugf("Station::wpsConfigStart() : cb failed\n");
		return false;
	}

	if(!wifi_wps_start()) {
		debugf("Station::wpsConfigStart() : wifi_wps_start() failed\n");
		return false;
	}
	return true;
}

void StationImpl::wpsConfigStop()
{
	if(!wifi_wps_disable()) {
		debugf("Station::wpsConfigStop() : wifi_wps_disable() failed\n");
	}
}

#endif // ENABLE_WPS
