/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AccessPoint.cpp
 *
 ****/

#include "Platform/AccessPoint.h"
#include "Interrupts.h"
#include "Data/HexString.h"

AccessPointClass WifiAccessPoint;

void AccessPointClass::enable(bool enabled, bool save)
{
	uint8 mode;
	if(save)
		mode = wifi_get_opmode_default() & ~SOFTAP_MODE;
	else
		mode = wifi_get_opmode() & ~SOFTAP_MODE;
	if(enabled)
		mode |= SOFTAP_MODE;
	if(save)
		wifi_set_opmode(mode);
	else
		wifi_set_opmode_current(mode);
}

bool AccessPointClass::isEnabled()
{
	return wifi_get_opmode() & SOFTAP_MODE;
}

bool AccessPointClass::config(const String& ssid, String password, AUTH_MODE mode, bool hidden, int channel,
							  int beaconInterval)
{
	softap_config config = {0};
	if(mode == AUTH_WEP)
		return false; // Not supported!

	if(mode == AUTH_OPEN)
		password = "";

	bool enabled = isEnabled();
	enable(true);
	wifi_softap_dhcps_stop();
	wifi_softap_get_config(&config);
	if(channel != config.channel || hidden != config.ssid_hidden || mode != config.authmode ||
	   beaconInterval != config.beacon_interval ||
	   strncmp(ssid.c_str(), (char*)config.ssid, sizeof(config.ssid)) != 0 ||
	   strncmp(password.c_str(), (char*)config.password, sizeof(config.password)) != 0) {
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
		if(System.isReady()) {
			noInterrupts();
			if(!wifi_softap_set_config(&config)) {
				interrupts();
				wifi_softap_dhcps_start();
				enable(enabled);
				debugf("Can't set AP configuration!");
				return false;
			}
			interrupts();
			debugf("AP configuration was updated");
		} else {
			debugf("Set AP configuration in background");
			if(runConfig != nullptr) {
				delete runConfig;
			}
			runConfig = new softap_config();
			memcpy(runConfig, &config, sizeof(softap_config));
		}
	} else {
		debugf("AP configuration loaded");
	}

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
	if(System.isReady()) {
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

String AccessPointClass::getMAC(char sep)
{
	uint8 hwaddr[6];
	if(wifi_get_macaddr(SOFTAP_IF, hwaddr))
		return makeHexString(hwaddr, sizeof(hwaddr), sep);
	else
		return nullptr;
}

String AccessPointClass::getSSID()
{
	softap_config config = {0};
	if(!wifi_softap_get_config(&config)) {
		debugf("Can't read AP configuration!");
		return "";
	}
	debugf("SSID: %s", (char*)config.ssid);
	return String((char*)config.ssid);
}

String AccessPointClass::getPassword()
{
	softap_config config = {0};
	if(!wifi_softap_get_config(&config)) {
		debugf("Can't read AP configuration!");
		return "";
	}
	debugf("Pass: %s", (char*)config.password);
	return String((char*)config.password);
}

void AccessPointClass::onSystemReady()
{
	if(runConfig != nullptr) {
		noInterrupts();
		bool enabled = isEnabled();
		enable(true);
		wifi_softap_dhcps_stop();

		if(!wifi_softap_set_config(runConfig)) {
			debugf("Can't set AP config on system ready event!");
		} else {
			debugf("AP configuration was updated on system ready event");
		}
		delete runConfig;
		runConfig = nullptr;

		wifi_softap_dhcps_start();
		enable(enabled);
		interrupts();
	}
}
