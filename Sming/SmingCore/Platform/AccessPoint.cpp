/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/*
 * 14/8/2018 (mikee47)
 *
 * 	In config() method if we fail to read the existing config we report it and
 * 	force the requested configuration change as a write may still succeed.
 *	We do not queue a system ready handler unless config has been called.
 *	The function has also been modified to simplify. It may seem less efficient,
 *	however normal behaviour is to change configuration
 *	 softap_config_equal() added.
 */

#include "AccessPoint.h"
#include "Interrupts.h"

// Limit number of connections to our AP
#define MAX_AP_CONNECTIONS 4

AccessPointClass WifiAccessPoint;

void AccessPointClass::enable(bool enabled, bool save)
{
	uint8 mode;
	if (save)
		mode = wifi_get_opmode_default() & ~SOFTAP_MODE;
	else
		mode = wifi_get_opmode() & ~SOFTAP_MODE;
	if (enabled)
		mode |= SOFTAP_MODE;
	if (save)
		wifi_set_opmode(mode);
	else
		wifi_set_opmode_current(mode);
}

/** @brief compare two AP configurations
 * 	@param lhs softap_config
 * 	@param rhs softap_config
 * 	@retval bool true if configs are the same
 * 	@note from ESP8266 Arduino source, changed to use safe string compare
 */
static bool softap_config_equal(const softap_config& lhs, const softap_config& rhs)
{
	if (strncmp(reinterpret_cast<const char*>(lhs.ssid), reinterpret_cast<const char*>(rhs.ssid), sizeof(lhs.ssid)) !=
		0)
		return false;

	if (strncmp(reinterpret_cast<const char*>(lhs.password), reinterpret_cast<const char*>(rhs.password),
				sizeof(lhs.password)) != 0)
		return false;

	if (lhs.channel != rhs.channel)
		return false;

	if (lhs.ssid_hidden != rhs.ssid_hidden)
		return false;

	if (lhs.max_connection != rhs.max_connection)
		return false;

	if (lhs.beacon_interval != rhs.beacon_interval)
		return false;

	if (lhs.authmode != rhs.authmode)
		return false;

	return true;
}

bool AccessPointClass::config(const String& ssid, String password, AUTH_MODE mode, bool hidden, int channel,
							  int beaconInterval)
{
	// WEP not supported
	if (mode == AUTH_WEP)
		return false;

	if (mode == AUTH_OPEN)
		password = nullptr;

	bool enabled = isEnabled();
	enable(true);
	wifi_softap_dhcps_stop();
	softap_config config = {0};
	bool cfgValid = wifi_softap_get_config(&config);
	if (!cfgValid)
		debugf("Failed to read soft AP config");

	/*
	 * We create the new config in anticipation of changing it using the onready handler.
	 * That is the most probable outcome.
	 */
	delete _runConfig;
	_runConfig = new softap_config();
	_runConfig->channel = channel;
	_runConfig->ssid_hidden = hidden;
	_runConfig->ssid_len = ssid.getBytes(_runConfig->ssid, sizeof(_runConfig->ssid));
	password.getBytes(_runConfig->password, sizeof(_runConfig->password));
	_runConfig->authmode = mode;
	_runConfig->max_connection = MAX_AP_CONNECTIONS;
	_runConfig->beacon_interval = beaconInterval;

	wifi_softap_dhcps_start();
	enable(enabled);

	if (cfgValid && softap_config_equal(config, *_runConfig)) {
		debugf("AP configuration loaded");
		delete _runConfig;
		_runConfig = nullptr;
	}
	else {
		// Note that this calls our handler immediately if the system is ready
		System.onReady(this);
	}

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
	if (System.isReady()) {
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
	uint8 hwaddr[6];
	if (wifi_get_macaddr(SOFTAP_IF, hwaddr))
		return toHexString(hwaddr, sizeof(hwaddr), ':');
	else
		return nullptr;
}

String AccessPointClass::getSSID()
{
	softap_config config = {0};
	if (!wifi_softap_get_config(&config)) {
		debugf("Can't read AP configuration!");
		return nullptr;
	}

	debugf("SSID: %s", (char*)config.ssid);
	return String((char*)config.ssid);
}

String AccessPointClass::getPassword()
{
	softap_config config = {0};
	if (!wifi_softap_get_config(&config)) {
		debugf("Can't read AP configuration!");
		return nullptr;
	}
	debugf("Pass: %s", (char*)config.password);
	return String((char*)config.password);
}

void AccessPointClass::onSystemReady()
{
	if (!_runConfig)
		return;

	noInterrupts();
	bool enabled = isEnabled();
	enable(true);
	wifi_softap_dhcps_stop();

	bool ret = wifi_softap_set_config(_runConfig);

	if (ret)
		debugf("AP configuration was updated");
	else
		debugf("Can't set AP configuration!");

	wifi_softap_dhcps_start();
	enable(enabled);
	interrupts();

	delete _runConfig;
	_runConfig = nullptr;
}
