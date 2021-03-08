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

#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_event.h>

static StationImpl station;
StationClass& WifiStation = station;

static esp_netif_t* stationNetworkInterface = nullptr;

class BssInfoImpl : public BssInfo
{
public:
	explicit BssInfoImpl(const wifi_ap_record_t* info)
	{
		ssid = reinterpret_cast<const char*>(info->ssid);
		bssid = info->bssid;
		authorization = (WifiAuthMode)info->authmode;
		channel = info->primary;
		rssi = info->rssi;
		hidden = 0;
	}
};

void StationImpl::enable(bool enabled, bool save)
{
	wifi_mode_t mode;
	ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
	if(enabled) {
		if(stationNetworkInterface == nullptr) {
			stationNetworkInterface = esp_netif_create_default_wifi_sta();
		}
		switch(mode) {
		case WIFI_MODE_STA:
		case WIFI_MODE_APSTA:
			return; // No change required
		case WIFI_MODE_AP:
			mode = WIFI_MODE_APSTA;
			break;
		case WIFI_MODE_NULL:
		default:
			mode = WIFI_MODE_STA;
		}
	} else {
		switch(mode) {
		case WIFI_MODE_NULL:
		case WIFI_MODE_AP:
			return; // No change required
		case WIFI_MODE_APSTA:
			mode = WIFI_MODE_AP;
			break;
		case WIFI_MODE_STA:
		default:
			mode = WIFI_MODE_NULL;
			break;
		}
		if(stationNetworkInterface != nullptr) {
			esp_netif_destroy(stationNetworkInterface);
			stationNetworkInterface = nullptr;
		}
	}
	ESP_ERROR_CHECK(esp_wifi_set_storage(save ? WIFI_STORAGE_FLASH : WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(mode));
}

bool StationImpl::isEnabled() const
{
	wifi_mode_t mode{};
	ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
	return (mode == WIFI_MODE_STA) || (mode == WIFI_MODE_APSTA);
}

bool StationImpl::config(const String& ssid, const String& password, bool autoConnectOnStartup, bool save)
{
	wifi_config_t config{};

	if(ssid.length() >= sizeof(config.sta.ssid)) {
		return false;
	}
	if(password.length() >= sizeof(config.sta.password)) {
		return false;
	}

	memcpy(config.sta.ssid, ssid.c_str(), ssid.length());
	memcpy(config.sta.password, password.c_str(), password.length());

	enable(true, save);

	ESP_ERROR_CHECK(esp_wifi_set_storage(save ? WIFI_STORAGE_FLASH : WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config));

	return connect();
}

bool StationImpl::connect()
{
	disconnect();
	return esp_wifi_start() == ESP_OK;
}

bool StationImpl::disconnect()
{
	esp_wifi_disconnect();
	return true;
}

bool StationImpl::isEnabledDHCP() const
{
	esp_netif_dhcp_status_t status;
	if(esp_netif_dhcps_get_status(stationNetworkInterface, &status) != ESP_OK) {
		return false;
	}

	return status == ESP_NETIF_DHCP_STARTED;
}

void StationImpl::enableDHCP(bool enable)
{
	if(enable) {
		esp_netif_dhcpc_start(stationNetworkInterface);
	} else {
		esp_netif_dhcpc_stop(stationNetworkInterface);
	}
}

void StationImpl::setHostname(const String& hostname)
{
	ESP_ERROR_CHECK(esp_netif_set_hostname(stationNetworkInterface, hostname.c_str()));
}

String StationImpl::getHostname() const
{
	const char* hostName;
	ESP_ERROR_CHECK(esp_netif_get_hostname(stationNetworkInterface, &hostName));
	return hostName;
}

IpAddress StationImpl::getIP() const
{
	IpAddress addr;
	esp_netif_ip_info_t info;
	if(esp_netif_get_ip_info(stationNetworkInterface, &info) == ESP_OK) {
		addr = info.ip.addr;
	}
	return addr;
}

MacAddress StationImpl::getMacAddress() const
{
	MacAddress addr;
	ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, &addr[0]));
	return addr;
}

bool StationImpl::setMacAddress(const MacAddress& addr) const
{
	return esp_wifi_set_mac(ESP_IF_WIFI_STA, &const_cast<MacAddress&>(addr)[0]);
}

IpAddress StationImpl::getNetworkBroadcast() const
{
	esp_netif_ip_info_t info;
	ESP_ERROR_CHECK(esp_netif_get_ip_info(stationNetworkInterface, &info));
	return info.ip.addr | ~info.netmask.addr;
}

IpAddress StationImpl::getNetworkMask() const
{
	esp_netif_ip_info_t info;
	ESP_ERROR_CHECK(esp_netif_get_ip_info(stationNetworkInterface, &info));
	return info.netmask.addr;
}

IpAddress StationImpl::getNetworkGateway() const
{
	esp_netif_ip_info_t info;
	ESP_ERROR_CHECK(esp_netif_get_ip_info(stationNetworkInterface, &info));
	return info.gw.addr;
}

bool StationImpl::setIP(IpAddress address, IpAddress netmask, IpAddress gateway)
{
	disconnect();
	enableDHCP(false);
	esp_netif_ip_info_t ipinfo;
	if(esp_netif_get_ip_info(stationNetworkInterface, &ipinfo) != ESP_OK) {
		return false;
	}
	ipinfo.ip.addr = address;
	ipinfo.netmask.addr = netmask;
	ipinfo.gw.addr = gateway;
	if(esp_netif_set_ip_info(stationNetworkInterface, &ipinfo) == ESP_OK) {
		debugf("Station IP successfully updated");
	} else {
		debugf("Station IP can't be updated");
		enableDHCP(true);
	}
	connect();
	return true;
}

String StationImpl::getSSID() const
{
	wifi_config_t config{};
	if(esp_wifi_get_config(ESP_IF_WIFI_STA, &config) != ESP_OK) {
		debugf("Can't read station configuration!");
		return nullptr;
	}
	auto ssid = reinterpret_cast<const char*>(config.sta.ssid);
	debugf("SSID: '%s'", ssid);
	return ssid;
}

int8_t StationImpl::getRssi() const
{
	wifi_ap_record_t info;
	ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&info));
	debugf("Rssi: %d dBm", info.rssi);
	return info.rssi;
}

uint8_t StationImpl::getChannel() const
{
	wifi_config_t config;
	if(esp_wifi_get_config(ESP_IF_WIFI_STA, &config) != ESP_OK) {
		debugf("Can't read station configuration!");
		return 0;
	}
	debugf("Channel: %d CH", config.sta.channel);
	return config.sta.channel;
}

String StationImpl::getPassword() const
{
	wifi_config_t config{};
	if(esp_wifi_get_config(ESP_IF_WIFI_STA, &config) != ESP_OK) {
		debugf("Can't read station configuration!");
		return nullptr;
	}
	auto pwd = reinterpret_cast<const char*>(config.sta.password);
	debugf("Pass: '%s'", pwd);
	return pwd;
}

StationConnectionStatus StationImpl::getConnectionStatus() const
{
	return WifiEventsImpl::stationConnectionStatus;
}

bool StationImpl::startScan(ScanCompletedDelegate scanCompleted)
{
	scanCompletedCallback = scanCompleted;
	if(!scanCompleted) {
		return false;
	}

	auto eventHandler = [](void* arg, esp_event_base_t base, int32_t id, void* data) {
		wifi_event_sta_scan_done_t* event = reinterpret_cast<wifi_event_sta_scan_done_t*>(data);
		staticScanCompleted(event, event->status);
	};

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, eventHandler, NULL));
	if(esp_wifi_scan_start(NULL, false) != ESP_OK) {
		auto connectHandler = [](void* arg, esp_event_base_t base, int32_t id, void* data) {
			esp_wifi_scan_start(NULL, false);
		};

		ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, connectHandler, NULL));

		debugf("startScan failed");
	}

	return true;
}

void StationImpl::staticScanCompleted(wifi_event_sta_scan_done_t* event, uint8_t status)
{
	BssList list;
	if(status == OK) {
		if(station.scanCompletedCallback) {
			uint16_t number = event->number;
			wifi_ap_record_t ap_info[number];
			uint16_t ap_count{0};
			memset(ap_info, 0, sizeof(ap_info));
			ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
			ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
			// TODO: Handle hidden APs
			for(unsigned i = 0; (i < event->number) && (i < ap_count); i++) {
				list.addElement(new BssInfoImpl(&ap_info[i]));
			}
			station.scanCompletedCallback(true, list);
		}

		debugf("scan completed: %u found", list.count());
	} else {
		debugf("scan failed %u", status);
		if(station.scanCompletedCallback) {
			station.scanCompletedCallback(false, list);
		}
	}
}

void StationImpl::onSystemReady()
{
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
