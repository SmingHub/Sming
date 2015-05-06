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
	onConnectOk = NULL;
	onConnectFail = NULL;
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
	enable(true);
	wifi_station_disconnect();
	wifi_station_dhcpc_stop();
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
		debugf("Can't set station configuration!");
		interrupts();
		wifi_station_connect();
		wifi_station_dhcpc_start();
		enable(enabled);
		return false;
	}
	debugf("Station configuration was updated to: %s", ssid.c_str());

	interrupts();
	wifi_station_connect();
	wifi_station_dhcpc_start();

	wifi_station_set_auto_connect(autoConnectOnStartup);
	enable(enabled);

	return true;
}

bool StationClass::isConnected()
{
	if (wifi_station_get_connect_status() != STATION_GOT_IP) return false;
	if (getIP().isNull()) return false;

	return true;
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

bool StationClass::setIP(IPAddress address)
{
	IPAddress gateway = IPAddress(address);
	gateway[3] = 1; // x.x.x.1
	setIP(address, IPAddress(255, 255, 255, 0), gateway);
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
	wifi_set_ip_info(STATION_IF, &ipinfo);
	wifi_station_connect();
	//wifi_station_dhcpc_start();
	return true;
}

bool StationClass::startScan(ScanCompletedCallback scanCompleted)
{
	scanCompletedCallback = scanCompleted;
	if (scanCompleted == NULL) return false;

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

void StationClass::waitConnection(ConnectionCallback successfulConnected)
{
	waitConnection(successfulConnected, -1, NULL);
}

void StationClass::waitConnection(ConnectionCallback successfulConnected, int secondsTimeOut, ConnectionCallback connectionNotEstablished)
{
	if (onConnectOk != NULL || onConnectFail != NULL)
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
		if (WifiStation.scanCompletedCallback != NULL)
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
		if (WifiStation.scanCompletedCallback != NULL)
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

void StationClass::checkConnection()
{
	uint32 duration = millis() - connectionStarted;
	if (isConnected())
	{
		ConnectionCallback call = onConnectOk;
		onConnectOk = NULL;
		onConnectFail = NULL;
		delete connectionTimer;
		connectionTimeOut = 0;

		if (call)
			call();

	}
	else if (connectionTimeOut > 0 && duration > (uint32)connectionTimeOut * 1000)
	{
		ConnectionCallback call = onConnectFail;
		onConnectOk = NULL;
		onConnectFail = NULL;
		delete connectionTimer;
		connectionTimeOut = 0;

		if (call)
			call();
	}
}

void StationClass::staticCheckConnection()
{
	WifiStation.checkConnection();
}

////////////

BssInfo::BssInfo(bss_info* info)
{
	ssid = String((char*)info->ssid);
	authorization = info->authmode;
	channel = info->channel;
	rssi = info->rssi;
	hidden = info->is_hidden;
}

bool BssInfo::isOpen()
{
	return authorization == AUTH_OPEN;
}

String BssInfo::getAuthorizationMethodName()
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
	}
}
