#include "../include/configuration.h"

#include <SmingCore.h>

MeteoConfig ActiveConfig;

MeteoConfig loadConfig()
{
	DynamicJsonDocument root(1024);
	MeteoConfig cfg;
	if(fileExist(METEO_CONFIG_FILE)) {
		int size = fileGetSize(METEO_CONFIG_FILE);
		char* jsonString = new char[size + 1];
		fileGetContent(METEO_CONFIG_FILE, jsonString, size + 1);

		auto error = deserializeJson(root, jsonString);
		if(!error) {
			JsonObject network = root["network"];
			cfg.NetworkSSID = String((const char*)network["ssid"]);
			cfg.NetworkPassword = String((const char*)network["password"]);

			JsonObject correction = root["correction"];
			cfg.AddT = correction["T"];
			cfg.AddRH = correction["RH"];
			cfg.AddTZ = correction["TZ"];

			JsonObject trigger = root["trigger"];
			cfg.Trigger = (TriggerType)(int)trigger["type"];
			cfg.RangeMin = trigger["min"];
			cfg.RangeMax = trigger["max"];
		}

		delete[] jsonString;
	} else {
		cfg.NetworkSSID = WIFI_SSID;
		cfg.NetworkPassword = WIFI_PWD;
	}
	return cfg;
}

void saveConfig(MeteoConfig& cfg)
{
	ActiveConfig = cfg;

	DynamicJsonDocument root(1024);
	auto network = root.createNestedObject("network");
	network["ssid"] = cfg.NetworkSSID.c_str();
	network["password"] = cfg.NetworkPassword.c_str();

	auto correction = root.createNestedObject("correction");
	correction["T"] = cfg.AddT;
	correction["RH"] = cfg.AddRH;
	correction["TZ"] = cfg.AddTZ;

	auto trigger = root.createNestedObject("trigger");
	trigger["type"] = (int)cfg.Trigger;
	trigger["min"] = cfg.RangeMin;
	trigger["max"] = cfg.RangeMax;

	char buf[3048];
	serializeJsonPretty(root, buf, sizeof(buf));
	fileSetContent(METEO_CONFIG_FILE, buf);
}
