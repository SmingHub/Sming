/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "AccessPoint.h"
#include "../../SmingCore/SmingCore.h"

AccessPointClass WifiAccessPoint;

AccessPointClass::AccessPointClass()
{
	System.onReady(this);
	runConfig = NULL;
}

void AccessPointClass::enable(bool enabled)
{
	uint8 mode = wifi_get_opmode() & ~SOFTAP_MODE;
	if (enabled) mode |= SOFTAP_MODE;
	wifi_set_opmode(mode);
}

bool AccessPointClass::isEnabled()
{
	return wifi_get_opmode() & SOFTAP_MODE;
}

bool AccessPointClass::config(String ssid, String password, AUTH_MODE mode, bool hidden /* = false*/, int channel /* = 7*/, int beaconInterval /* = 200*/)
{
	softap_config config = {0};
	if (mode == AUTH_WEP) return false; // Not supported!

	if (mode == AUTH_OPEN)
		password = "";

	bool enabled = isEnabled();
	enable(true);
	wifi_softap_dhcps_stop();
	wifi_softap_get_config(&config);
	config.channel = channel;
	config.ssid_hidden = hidden;
	memset(config.ssid, 0, sizeof(config.ssid));
	memset(config.password, 0, sizeof(config.password));
	strcpy((char*)config.ssid, ssid.c_str());
	strcpy((char*)config.password, password.c_str());
	config.ssid_len = ssid.length();
	config.authmode = mode;
	config.max_connection = 4; // max 4
	config.beacon_interval = beaconInterval;
	if (System.isReady())
	{
		noInterrupts();
		if (!wifi_softap_set_config(&config))
		{
			interrupts();
			wifi_softap_dhcps_start();
			enable(enabled);
			debugf("Can't set AP configuration!");
			return false;
		}
		interrupts();
		debugf("AP configuration was updated");
	}
	else
	{
		debugf("Set AP configuration in background");
		if (runConfig != NULL)
			delete runConfig;
		runConfig = new softap_config();
		memcpy(runConfig, &config, sizeof(softap_config));
	}

	wifi_softap_dhcps_start();
	enable(enabled);

	return true;
}

void AccessPointClass::disconnect()
{
	wifi_station_disconnect();
}

IPAddress AccessPointClass::getIP()
{
	struct ip_info info = {0};
	wifi_get_ip_info(SOFTAP_IF, &info);
	return info.ip;
}

bool AccessPointClass::setIP(IPAddress address)
{
	if (System.isReady())
	{
		debugf("IP can be changed only in init() method");
		return false;
	}

	wifi_softap_dhcps_stop();
	struct ip_info ipinfo;
	wifi_get_ip_info(SOFTAP_IF, &ipinfo);
	ipinfo.ip = address;
	ipinfo.gw = address;
	IP4_ADDR(&ipinfo.netmask, 255, 255, 255, 0);
	wifi_set_ip_info(SOFTAP_IF, &ipinfo);
	wifi_softap_dhcps_start();
	return true;
}

String AccessPointClass::getMAC()
{
	String mac;
	uint8 hwaddr[6] = {0};
	wifi_get_macaddr(SOFTAP_IF, hwaddr);
	for (int i = 0; i < 6; i++)
	{
		if (hwaddr[i] < 0x10) mac += "0";
		mac += String(hwaddr[i], HEX);
	}
	return mac;
}

void AccessPointClass::onSystemReady()
{
	if (runConfig != NULL)
	{
		noInterrupts();
		bool enabled = isEnabled();
		enable(true);
		wifi_softap_dhcps_stop();

		if(!wifi_softap_set_config(runConfig))
			debugf("Can't set AP config on system ready event!");
		else
			debugf("AP configuration was updated on system ready event");
		delete runConfig;
		runConfig = NULL;

		wifi_softap_dhcps_start();
		enable(enabled);
		interrupts();
	}
}
