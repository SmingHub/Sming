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

#ifdef ENABLE_WPS
#include <esp_wps.h>

/*
 * Information only required during WPS negotiation
 */
struct StationImpl::WpsConfig {
	static constexpr unsigned timeoutMs{60000};
	static constexpr unsigned maxRetryAttempts{5};
	WPSConfigDelegate callback;
	wifi_event_sta_wps_er_success_t creds;
	uint8_t numRetries;
	uint8_t credIndex;
	bool ignoreDisconnects;
};

#endif

static StationImpl station;
StationClass& WifiStation = station;

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
	if(stationNetworkInterface == nullptr) {
		return false;
	}
	esp_netif_dhcp_status_t status;
	if(esp_netif_dhcps_get_status(stationNetworkInterface, &status) != ESP_OK) {
		return false;
	}

	return status == ESP_NETIF_DHCP_STARTED;
}

void StationImpl::enableDHCP(bool enable)
{
	if(stationNetworkInterface == nullptr) {
		return;
	}
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
	if(stationNetworkInterface == nullptr) {
		return false;
	}
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
		debug_i("Station IP successfully updated");
	} else {
		debug_e("Station IP can't be updated");
		enableDHCP(true);
	}
	connect();
	return true;
}

String StationImpl::getSSID() const
{
	wifi_config_t config{};
	if(esp_wifi_get_config(ESP_IF_WIFI_STA, &config) != ESP_OK) {
		debug_e("Can't read station configuration!");
		return nullptr;
	}
	auto ssid = reinterpret_cast<const char*>(config.sta.ssid);
	debug_d("SSID: '%s'", ssid);
	return ssid;
}

int8_t StationImpl::getRssi() const
{
	wifi_ap_record_t info;
	ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&info));
	debug_d("Rssi: %d dBm", info.rssi);
	return info.rssi;
}

uint8_t StationImpl::getChannel() const
{
	wifi_config_t config;
	if(esp_wifi_get_config(ESP_IF_WIFI_STA, &config) != ESP_OK) {
		debug_e("Can't read station configuration!");
		return 0;
	}
	debug_d("Channel: %d CH", config.sta.channel);
	return config.sta.channel;
}

String StationImpl::getPassword() const
{
	wifi_config_t config{};
	if(esp_wifi_get_config(ESP_IF_WIFI_STA, &config) != ESP_OK) {
		debug_e("Can't read station configuration!");
		return nullptr;
	}
	auto pwd = reinterpret_cast<const char*>(config.sta.password);
	debug_d("Pass: '%s'", pwd);
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

		debug_e("startScan failed");
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

		debug_i("scan completed: %u found", list.count());
	} else {
		debug_e("scan failed %u", status);
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

void StationImpl::wpsEventHandler(esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if(wpsConfig == nullptr) {
		return;
	}

	switch(event_id) {
	case WIFI_EVENT_STA_DISCONNECTED:
		debug_w("WIFI_EVENT_STA_DISCONNECTED");
		if(wpsConfig->ignoreDisconnects) {
			break;
		}
		if(wpsConfig->numRetries < WpsConfig::maxRetryAttempts) {
			esp_wifi_connect();
			++wpsConfig->numRetries;
			break;
		}

		if(wpsConfigure(wpsConfig->credIndex + 1)) {
			esp_wifi_connect();
			break;
		}

		debug_e("[WPS] Failed to connect!");
		if(wpsCallback(WpsStatus::Failed)) {
			// try to reconnect with old config
			wpsConfigStop();
			esp_wifi_connect();
		}
		break;

	case WIFI_EVENT_STA_WPS_ER_SUCCESS: {
		debug_i("WIFI_EVENT_STA_WPS_ER_SUCCESS");

		if(!wpsCallback(WpsStatus::Success)) {
			return;
		}

		if(event_data != nullptr) {
			/* If multiple AP credentials are received from WPS, connect with first one */
			wpsConfig->creds = *static_cast<wifi_event_sta_wps_er_success_t*>(event_data);
			wpsConfigure(0);
		}
		/*
		 * If only one AP credential is received from WPS, there will be no event data and
		 * esp_wifi_set_config() is already called by WPS modules for backward compatibility
		 * with legacy apps. So directly attempt connection here.
		 */
		wpsConfigStop();
		esp_wifi_connect();
		break;
	}

	case WIFI_EVENT_STA_WPS_ER_FAILED: {
		debug_e("WIFI_EVENT_STA_WPS_ER_FAILED");
		if(wpsCallback(WpsStatus::Failed)) {
			// Try to reconnect with old config
			wpsConfigStop();
			esp_wifi_connect();
		}
		break;
	}

	case WIFI_EVENT_STA_WPS_ER_TIMEOUT:
		debug_e("WIFI_EVENT_STA_WPS_ER_TIMEOUT");
		if(wpsCallback(WpsStatus::Timeout)) {
			// Try to reconnect with old config
			wpsConfigStop();
			esp_wifi_connect();
		}
		break;

	case WIFI_EVENT_STA_WPS_ER_PIN:
		debug_e("WIFI_EVENT_STA_WPS_ER_PIN (not implemented)");
		break;

	default:
		break;
	}
}

bool StationImpl::wpsConfigure(uint8_t credIndex)
{
	wpsConfig->ignoreDisconnects = false;
	if(credIndex >= wpsConfig->creds.ap_cred_cnt) {
		return false;
	}
	wpsConfig->numRetries = 0;
	wpsConfig->credIndex = credIndex;
	auto& cred = wpsConfig->creds.ap_cred[credIndex];
	debug_i("Connecting to SSID: %s, Passphrase: %s", cred.ssid, cred.passphrase);
	wifi_config_t cfg{};
	memcpy(cfg.sta.ssid, cred.ssid, sizeof(cred.ssid));
	memcpy(cfg.sta.password, cred.passphrase, sizeof(cred.passphrase));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
	return true;
}

bool StationImpl::wpsConfigStart(WPSConfigDelegate callback)
{
	if(wpsConfig != nullptr) {
		debug_e("[WPS] Already in progress");
		return false;
	}

	wpsConfig = new WpsConfig{};
	wpsConfig->callback = callback;
	wpsConfig->ignoreDisconnects = true;

	debug_d("[WPS] wpsConfigStart()");

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, staticWpsEventHandler, this));

	enable(true, false);

	connect();

	esp_wps_config_t wps_config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);
	ESP_ERROR_CHECK(esp_wifi_wps_enable(&wps_config));
	ESP_ERROR_CHECK(esp_wifi_wps_start(WpsConfig::timeoutMs));

	return true;
}

bool StationImpl::wpsCallback(WpsStatus status)
{
	return wpsConfig->callback ? wpsConfig->callback(status) : true;
}

void StationImpl::wpsConfigStop()
{
	ESP_ERROR_CHECK(esp_wifi_wps_disable());
	ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, staticWpsEventHandler));
	delete wpsConfig;
	wpsConfig = nullptr;
}

#endif // ENABLE_WPS
