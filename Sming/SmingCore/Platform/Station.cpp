/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "Station.h"
#include "../../SmingCore/SmingCore.h"

StationClass WifiStation;

StationClass::StationClass()
{
	System.onReady(this);
	runScan = false;
	onConnectOk = nullptr;
	onConnectFail = nullptr;
	connectionTimeOut = 0;
	connectionTimer = NULL;
}

StationClass::~StationClass()
{
	delete connectionTimer;
	connectionTimer = NULL;
}

void StationClass::enable(bool enabled)
{
	uint8 mode = wifi_get_opmode() & ~STATION_MODE;
	if (enabled) mode |= STATION_MODE;
	wifi_set_opmode(mode);
}

bool StationClass::isEnabled()
{
	return wifi_get_opmode() & STATION_MODE;
}

bool StationClass::config(String ssid, String password, bool autoConnectOnStartup /* = true*/)
{
	station_config config = {0};

	if (ssid.length() >= sizeof(config.ssid)) return false;
	if (password.length() >= sizeof(config.password)) return false;

	bool enabled = isEnabled();
	bool dhcp = isEnabledDHCP();
	enable(true); // Power on for configuration

	wifi_station_disconnect();
	if (dhcp) enableDHCP(false);
	bool cfgreaded = wifi_station_get_config(&config);
	if (!cfgreaded) debugf("Can't read station configuration!");

	memset(config.ssid, 0, sizeof(config.ssid));
	memset(config.password, 0, sizeof(config.password));
	config.bssid_set = false;
	strcpy((char*)config.ssid, ssid.c_str());
	strcpy((char*)config.password, password.c_str());

	noInterrupts();
	if(!wifi_station_set_config(&config))
	{
		interrupts();
		debugf("Can't set station configuration!");
		wifi_station_connect();
		enableDHCP(dhcp);
		enable(enabled);
		return false;
	}
	debugf("Station configuration was updated to: %s", ssid.c_str());

	interrupts();
	wifi_station_connect();
	enableDHCP(dhcp);
	enable(enabled);

	wifi_station_set_auto_connect(autoConnectOnStartup);

	return true;
}

void StationClass::disconnect()
{
	wifi_station_disconnect();
}

bool StationClass::isConnected()
{
	if (getConnectionStatus() != eSCS_GotIP) return false;
	if (getIP().isNull()) return false;

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
	if (enable)
		wifi_station_dhcpc_start();
	else
		wifi_station_dhcpc_stop();
}

IPAddress StationClass::getIP()
{
	struct ip_info info = {0};
	wifi_get_ip_info(STATION_IF, &info);
	return info.ip;
}

String StationClass::getMAC()
{
	String mac;
	uint8 hwaddr[6] = {0};
	wifi_get_macaddr(STATION_IF, hwaddr);
	for (int i = 0; i < 6; i++)
	{
		if (hwaddr[i] < 0x10) mac += "0";
		mac += String(hwaddr[i], HEX);
	}
	return mac;
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
	setIP(address, mask, gateway);
}

bool StationClass::setIP(IPAddress address, IPAddress netmask, IPAddress gateway)
{
	if (System.isReady())
	{
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
		debugf("AP IP succesfully updated");
	else
	{
		debugf("AP IP can't be updated");
		enableDHCP(true);
	}
	wifi_station_connect();
	//wifi_station_dhcpc_start();
	return true;
}

String StationClass::getSSID()
{
	station_config config = {0};
	if (!wifi_station_get_config(&config))
	{
		debugf("Can't read station configuration!");
		return "";
	}
	debugf("SSID: %s", (char*)config.ssid);
	return String((char*)config.ssid);
}

String StationClass::getPassword()
{
	station_config config = {0};
	if (!wifi_station_get_config(&config))
	{
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
	if (!scanCompleted) return false;

	bool res = wifi_station_scan(NULL, staticScanCompleted);
	if (!res)
	{
		if (!System.isReady())
		{
			// It's OK, queue this task
			runScan = true;
			return true;
		}
		debugf("startScan failed");
	}
	return res;
}

void StationClass::waitConnection(ConnectionDelegate successfulConnected)
{
	waitConnection(successfulConnected, -1, NULL);
}

void StationClass::waitConnection(ConnectionDelegate successfulConnected, int secondsTimeOut, ConnectionDelegate connectionNotEstablished)
{
	if (onConnectOk || onConnectFail )
	{
		SYSTEM_ERROR("WRONG CALL waitConnection method..");
		return;
	}

	onConnectOk = successfulConnected;
	connectionTimeOut = secondsTimeOut;
	onConnectFail = connectionNotEstablished;
	connectionTimer = new Timer();
	connectionTimer->initializeMs(50, staticCheckConnection).start();
	connectionStarted = millis();
}

////////////

void StationClass::staticScanCompleted(void *arg, STATUS status)
{
	BssList list;
	if (status == OK)
	{
		if (WifiStation.scanCompletedCallback )
		{
			bss_info *cur = (bss_info*)arg;

			while (cur != NULL)
			{
				list.add(BssInfo(cur));
				cur = cur->next.stqe_next;
			}
			WifiStation.scanCompletedCallback(true, list);
		}

		debugf("scan completed: %d found", list.count());
	}
	else
	{
		debugf("scan failed %d", status);
		if (WifiStation.scanCompletedCallback )
			WifiStation.scanCompletedCallback(false, list);
	}
}

void StationClass::onSystemReady()
{
	if (runScan)
	{
		wifi_station_scan(NULL, staticScanCompleted);
		runScan = false;
	}
}

void StationClass::internalCheckConnection()
{
	uint32 duration = millis() - connectionStarted;
	if (isConnected())
	{
		ConnectionDelegate callOk = nullptr;
		if (onConnectOk) {
			callOk = onConnectOk;
		}

		onConnectOk = nullptr;
		onConnectFail = nullptr;
		delete connectionTimer;
		connectionTimeOut = 0;

		if (callOk) {
			callOk();
		}
	}
	else if (connectionTimeOut > 0 && duration > (uint32)connectionTimeOut * 1000)
	{
		ConnectionDelegate call = onConnectFail;
		onConnectOk = nullptr;
		onConnectFail = nullptr;
		delete connectionTimer;
		connectionTimeOut = 0;

		if (call)
			call();
	}
}

void StationClass::staticCheckConnection()
{
	WifiStation.internalCheckConnection();
}

const char* StationClass::getConnectionStatusName()
{
	switch (getConnectionStatus())
	{
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

void smartConfigDone(sc_status status, void *pdata) {

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
		case SC_STATUS_LINK:
			{
				debugf("SC_STATUS_LINK\n");
				station_config *sta_conf = (station_config *)pdata;
				char *ssid = (char*)sta_conf->ssid;
				char *password = (char*)sta_conf->password;
				WifiStation.config(ssid, password);
			}
			break;
		case SC_STATUS_LINK_OVER:
			debugf("SC_STATUS_LINK_OVER\n");
			WifiStation.smartConfigStop();
			break;
	}
}

void StationClass::smartConfigStart(SmartConfigType sctype) {
	smartconfig_set_type((sc_type)sctype);
	smartconfig_start(smartConfigDone);
}

void StationClass::smartConfigStop() {
	smartconfig_stop();
}

////////////

BssInfo::BssInfo(bss_info* info)
{
	ssid = String((char*)info->ssid);
	memcpy(bssid, info->bssid, sizeof(bssid));
	authorization = info->authmode;
	channel = info->channel;
	rssi = info->rssi;
	hidden = info->is_hidden;
}

bool BssInfo::isOpen()
{
	return authorization == AUTH_OPEN;
}

const char* BssInfo::getAuthorizationMethodName()
{
	switch (authorization)
	{
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
	uint32_t a = *(uint16_t*)(&bssid[4]);
	uint32_t b = *(uint32_t*)bssid;
	return a ^ b;
}
