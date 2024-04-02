#include "configuration.h"
#include <ArduinoJson.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
DEFINE_FSTR(KEY_NETWORK, "network")
DEFINE_FSTR(KEY_SSID, "StaSSID")
DEFINE_FSTR(KEY_PASS, "StaPassword")
DEFINE_FSTR(KEY_ENABLE, "StaEnable")
} // namespace

ThermConfig activeConfig;

ThermConfig loadConfig()
{
	StaticJsonDocument<ConfigJsonBufferSize> doc;
	ThermConfig cfg;
	if(Json::loadFromFile(doc, THERM_CONFIG_FILE)) {
		JsonObject network = doc[KEY_NETWORK];
		cfg.StaSSID = network[KEY_SSID].as<const char*>();
		cfg.StaPassword = network[KEY_PASS].as<const char*>();
		cfg.StaEnable = network[KEY_ENABLE];
	} else {
		// Factory defaults if no config file present, or could not access it
		cfg.StaSSID = F(WIFI_SSID);
		cfg.StaPassword = F(WIFI_PWD);
	}
	return cfg;
}

void saveConfig(ThermConfig& cfg)
{
	StaticJsonDocument<ConfigJsonBufferSize> doc;

	JsonObject network = doc.createNestedObject(KEY_NETWORK);
	network[KEY_SSID] = cfg.StaSSID;
	network[KEY_PASS] = cfg.StaPassword;
	network[KEY_ENABLE] = cfg.StaEnable;

	Json::saveToFile(doc, THERM_CONFIG_FILE, Json::Pretty);
}
