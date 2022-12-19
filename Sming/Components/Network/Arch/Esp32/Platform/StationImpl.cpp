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
#include <nvs.h>

#ifdef ENABLE_WPS
#include <esp_wps.h>
#endif

// Use same NVS namespace as other WiFi settings
#define NVS_NAMESPACE "nvs.net80211"
#define NVS_STA_AUTOCONNECT "sta.autoconnect"

StationClass& WifiStation{SmingInternal::Network::station};

namespace SmingInternal
{
namespace Network
{
StationImpl station;

#ifdef ENABLE_WPS
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

void setAutoConnect(bool enable)
{
	nvs_handle_t handle;
	ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle));
	nvs_set_u8(handle, NVS_STA_AUTOCONNECT, enable);
	nvs_close(handle);
}

bool getAutoConnect()
{
	uint8_t enable{false};
	nvs_handle_t handle;
	if(nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) == ESP_OK) {
		nvs_get_u8(handle, NVS_STA_AUTOCONNECT, &enable);
		nvs_close(handle);
	}
	return enable;
}

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

void StationImpl::eventHandler(esp_event_base_t base, int32_t id, void* data)
{
	if(base == WIFI_EVENT) {
		bool allowAutoConnect{true};
#ifdef ENABLE_WPS
		if(wpsConfig != nullptr) {
			wpsEventHandler(id, data);
			allowAutoConnect = false;
		}
#endif
#ifdef ENABLE_SMART_CONFIG
		if(smartConfigEventInfo) {
			allowAutoConnect = false;
		}
#endif
		switch(id) {
		case WIFI_EVENT_STA_START:
			if(allowAutoConnect && getAutoConnect()) {
				connectionStatus = eSCS_Connecting;
				esp_wifi_connect();
			}
			break;
		case WIFI_EVENT_STA_DISCONNECTED: {
			connectionStatus = eSCS_ConnectionFailed;
			break;
		}
		default:;
		}
	} else if(base == IP_EVENT) {
		switch(id) {
		case IP_EVENT_STA_GOT_IP:
			connectionStatus = eSCS_GotIP;
			break;
		case IP_EVENT_STA_LOST_IP:
			connectionStatus = eSCS_Connecting;
			break;
		default:;
		}
	}
}

void StationImpl::enable(bool enabled, bool save)
{
	wifi_mode_t mode;
	ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
	if(enabled) {
		if(!stationNetworkInterface) {
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
	ESP_ERROR_CHECK(esp_wifi_start());
}

bool StationImpl::isEnabled() const
{
	wifi_mode_t mode{};
	ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
	return (mode == WIFI_MODE_STA) || (mode == WIFI_MODE_APSTA);
}

bool StationImpl::config(const Config& cfg)
{
	wifi_config_t config{};

	if(cfg.ssid.length() >= sizeof(config.sta.ssid)) {
		return false;
	}
	if(cfg.password.length() >= sizeof(config.sta.password)) {
		return false;
	}

	memcpy(config.sta.ssid, cfg.ssid.c_str(), cfg.ssid.length());
	memcpy(config.sta.password, cfg.password.c_str(), cfg.password.length());

	if(cfg.bssid) {
		config.sta.bssid_set = true;
		cfg.bssid.getOctets(config.sta.bssid);
	} else {
		config.sta.bssid_set = false;
	}

	// Find *all* APs for the requested SSID and pick the best one
	config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
	config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;

	enable(true, cfg.save);

	if(cfg.save) {
		setAutoConnect(cfg.autoConnectOnStartup);
	}

	ESP_ERROR_CHECK(esp_wifi_set_storage(cfg.save ? WIFI_STORAGE_FLASH : WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));

	return connect();
}

bool StationImpl::connect()
{
	disconnect();
	return esp_wifi_connect() == ESP_OK;
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
	ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, &addr[0]));
	return addr;
}

bool StationImpl::setMacAddress(const MacAddress& addr) const
{
	return esp_wifi_set_mac(WIFI_IF_STA, &const_cast<MacAddress&>(addr)[0]);
}

IpAddress StationImpl::getNetworkBroadcast() const
{
	IpAddress addr;
	esp_netif_ip_info_t info;
	if(esp_netif_get_ip_info(stationNetworkInterface, &info) == ESP_OK) {
		addr = info.ip.addr | ~info.netmask.addr;
	}
	return addr;
}

IpAddress StationImpl::getNetworkMask() const
{
	IpAddress addr;
	esp_netif_ip_info_t info;
	if(esp_netif_get_ip_info(stationNetworkInterface, &info) == ESP_OK) {
		addr = info.netmask.addr;
	}
	return addr;
}

IpAddress StationImpl::getNetworkGateway() const
{
	IpAddress addr;
	esp_netif_ip_info_t info;
	if(esp_netif_get_ip_info(stationNetworkInterface, &info) == ESP_OK) {
		addr = info.gw.addr;
	}
	return addr;
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
	if(esp_wifi_get_config(WIFI_IF_STA, &config) != ESP_OK) {
		debug_e("Can't read station configuration!");
		return nullptr;
	}
	auto ssid = reinterpret_cast<const char*>(config.sta.ssid);
	debug_d("SSID: '%s'", ssid);
	return ssid;
}

MacAddress StationImpl::getBSSID() const
{
	wifi_config_t config{};
	if(esp_wifi_get_config(WIFI_IF_STA, &config) != ESP_OK) {
		debug_e("Can't read station configuration!");
		return MacAddress{};
	}
	return config.sta.bssid;
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
	if(esp_wifi_get_config(WIFI_IF_STA, &config) != ESP_OK) {
		debug_e("Can't read station configuration!");
		return 0;
	}
	debug_d("Channel: %d CH", config.sta.channel);
	return config.sta.channel;
}

String StationImpl::getPassword() const
{
	wifi_config_t config{};
	if(esp_wifi_get_config(WIFI_IF_STA, &config) != ESP_OK) {
		debug_e("Can't read station configuration!");
		return nullptr;
	}
	auto pwd = reinterpret_cast<const char*>(config.sta.password);
	debug_d("Pass: '%s'", pwd);
	return pwd;
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

void StationImpl::internalSmartConfig(smartconfig_event_t event_id, void* pdata)
{
	if(!smartConfigEventInfo) {
		debug_e("[SC] ERROR! eventInfo null");
		return;
	}

	auto& evt = *smartConfigEventInfo;

	SmartConfigEvent event;
	switch(event_id) {
	case SC_EVENT_SCAN_DONE:
		debugf("[SC] SCAN_DONE");
		event = SCE_FindChannel;
		break;
	case SC_EVENT_FOUND_CHANNEL:
		debugf("[SC] FOUND_CHANNEL");
		event = SCE_GettingSsid;
		break;
	case SC_EVENT_SEND_ACK_DONE:
		debugf("[SC] SEND_ACK_DONE");
		event = SCE_LinkOver;
		break;
	case SC_EVENT_GOT_SSID_PSWD: {
		debugf("[SC] GOT_SSID_PSWD");
		auto cfg = static_cast<const smartconfig_event_got_ssid_pswd_t*>(pdata);
		assert(cfg != nullptr);
		if(cfg == nullptr) {
			return;
		}
		evt.ssid = reinterpret_cast<const char*>(cfg->ssid);
		evt.password = reinterpret_cast<const char*>(cfg->password);
		evt.bssidSet = cfg->bssid_set;
		evt.bssid = cfg->bssid;
		evt.type = SmartConfigType(cfg->type);
		event = SCE_Link;
		break;
	}
	default:
		debugf("[SC] UNKNOWN %u", event_id);
		return;
	}

	if(smartConfigCallback && !smartConfigCallback(event, evt)) {
		return;
	}

	switch(event_id) {
	case SC_EVENT_GOT_SSID_PSWD:
		StationClass::config(evt.ssid, evt.password, true, true);
		connect();
		break;
	case SC_EVENT_SEND_ACK_DONE:
		smartConfigStop();
		break;
	default:;
	}
}

void StationImpl::smartConfigEventHandler(void* arg, esp_event_base_t base, int32_t id, void* data)
{
	auto self = static_cast<StationImpl*>(arg);
	return self->internalSmartConfig(smartconfig_event_t(id), data);
}

bool StationImpl::smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback)
{
	if(smartConfigEventInfo) {
		return false; // Already in progress
	}

	if(esp_smartconfig_set_type(smartconfig_type_t(sctype)) != ESP_OK) {
		debug_e("smartconfig_set_type(%u) failed", sctype);
		return false;
	}

	smartConfigEventInfo.reset(new SmartConfigEventInfo{});
	if(!smartConfigEventInfo) {
		return false;
	}

	smartConfigCallback = callback;
	ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, smartConfigEventHandler, this));

	smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
	if(esp_smartconfig_start(&cfg) != ESP_OK) {
		debug_e("esp_smartconfig_start() failed");
		esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, smartConfigEventHandler);
		smartConfigCallback = nullptr;
		smartConfigEventInfo.reset();
		return false;
	}

	return true;
}

void StationImpl::smartConfigStop()
{
	esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, smartConfigEventHandler);
	esp_smartconfig_stop();
	smartConfigCallback = nullptr;
	smartConfigEventInfo.reset();
}

#endif // ENABLE_SMART_CONFIG

#ifdef ENABLE_WPS

void StationImpl::wpsEventHandler(int32_t event_id, void* event_data)
{
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
	delete wpsConfig;
	wpsConfig = nullptr;
}

#endif // ENABLE_WPS

}; // namespace Network
}; // namespace SmingInternal
