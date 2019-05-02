#include <tytherm.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

ThermConfig ActiveConfig;

ThermConfig loadConfig()
{
	StaticJsonDocument<ConfigJsonBufferSize> doc;
	ThermConfig cfg;
	if(Json::loadFromFile(doc, THERM_CONFIG_FILE)) {
		JsonObject network = doc["network"];
		cfg.StaSSID = network["StaSSID"].as<const char*>();
		cfg.StaPassword = network["StaPassword"].as<const char*>();
		cfg.StaEnable = network["StaEnable"];
	} else {
		//Factory defaults if no config file present, or could not access it
		cfg.StaSSID = WIFI_SSID;
		cfg.StaPassword = WIFI_PWD;
	}
	return cfg;
}

void saveConfig(ThermConfig& cfg)
{
	StaticJsonDocument<ConfigJsonBufferSize> doc;

	JsonObject network = doc.createNestedObject("network");
	network["StaSSID"] = cfg.StaSSID;
	network["StaPassword"] = cfg.StaPassword;
	network["StaEnable"] = cfg.StaEnable;

	Json::saveToFile(doc, THERM_CONFIG_FILE, Json::Pretty);
}
