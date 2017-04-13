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

void AccessPointClass::enable(bool enabled, bool save)
{
	uint8 mode;
	if (save)
		mode = wifi_get_opmode_default() & ~SOFTAP_MODE;
	else
		mode = wifi_get_opmode() & ~SOFTAP_MODE;
	if (enabled) mode |= SOFTAP_MODE;
	if (save)
		wifi_set_opmode(mode);
	else
		wifi_set_opmode_current(mode);
}

bool AccessPointClass::isEnabled()
{
	return wifi_get_opmode() & SOFTAP_MODE;
}

bool AccessPointClass::config(String ssid, String password, AUTH_MODE mode, bool hidden /* = false*/, int channel /* = 7*/, int beaconInterval /* = 200*/)
{
	config(ssid.c_str(), password.c_str(), mode, hidden, channel, beaconInterval);
}

bool AccessPointClass::config(const char * ssid, const char * password, AUTH_MODE mode, bool hidden /* = false*/, int channel /* = 7*/, int beaconInterval /* = 200*/)
{
	softap_config config = {0};
	if (mode == AUTH_WEP) return false; // Not supported!

	char pwd[64];
	if (mode == AUTH_OPEN)
		pwd[0] = '\0';
	else
		strncpy(pwd, password, 63);

	bool enabled = isEnabled();
	enable(true);
	wifi_softap_dhcps_stop();
	wifi_softap_get_config(&config);
	if (channel != config.channel || hidden != config.ssid_hidden
		|| mode != config.authmode|| beaconInterval != config.beacon_interval
		|| strncmp(ssid, (char*)config.ssid, sizeof(config.ssid))!=0 || strncmp(pwd, (char*)config.password, sizeof(config.password))!=0)
	{
		config.channel = channel;
		config.ssid_hidden = hidden;
		memset(config.ssid, 0, sizeof(config.ssid));
		memset(config.password, 0, sizeof(config.password));
		strcpy((char*)config.ssid, ssid);
		strcpy((char*)config.password, pwd);
		config.ssid_len = strlen(ssid);
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
	}
	else
		debugf("AP configuration loaded");

	wifi_softap_dhcps_start();
	enable(enabled);

	return true;
}

IPAddress AccessPointClass::getIP()
{
	struct ip_info info = {0};
	wifi_get_ip_info(SOFTAP_IF, &info);
	return info.ip;
}

IPAddress AccessPointClass::getNetworkBroadcast()
{
	struct ip_info info = {0};
	wifi_get_ip_info(SOFTAP_IF, &info);
	return (info.ip.addr | ~info.netmask.addr);
}

IPAddress AccessPointClass::getNetworkMask()
{
	struct ip_info info = {0};
	wifi_get_ip_info(SOFTAP_IF, &info);
	return info.netmask;
}

IPAddress AccessPointClass::getNetworkGateway()
{
	struct ip_info info = {0};
	wifi_get_ip_info(SOFTAP_IF, &info);
	return info.gw;
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
	char macBuf[6 * 2 + 1];
	return String(getMAC(macBuf, sizeof(macBuf)));
}

char * AccessPointClass::getMAC(char * s, size_t bufSize)
{
	uint8 hwaddr[6] = {0};
	wifi_get_macaddr(SOFTAP_IF, hwaddr);
	m_snprintf(s, bufSize - 1, "%02x%02x%02x%02x%02x%02x", hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
	return s;	
}

uint8 * AccessPointClass::getMAC(uint8 mac[6])
{
	wifi_get_macaddr(SOFTAP_IF, mac);
	return mac;
}

String AccessPointClass::getSSID()
{
	char buf[33];
	return String(getSSID(buf, sizeof(buf)));
}

char * AccessPointClass::getSSID(char * s, size_t bufSize)
{
	softap_config config = {0};
	if (!wifi_softap_get_config(&config))
	{
		debugf("Can't read AP configuration!");
	}
	else
	{
		debugf("SSID: %s", (char*)config.ssid);
		strncpy(s, (char*)config.ssid, bufSize - 1);
		s[bufSize - 1] = '\0';
	}
	return s;
}

String AccessPointClass::getPassword()
{
	char buf[64];
	return String(getPassword(buf, sizeof(buf))); 
}

char * AccessPointClass::getPassword(char * s, size_t bufSize)
{
	softap_config config = {0};
	if (!wifi_softap_get_config(&config))
	{
		debugf("Can't read AP configuration!");
	}
	else
	{
		debugf("Pass: %s", (char*)config.password);
		strncpy(s, (char*)config.password, bufSize - 1);
		s[bufSize - 1] = '\0';
	}
	return s;
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
