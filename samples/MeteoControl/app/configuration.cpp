#include "configuration.h"

#include <SmingCore.h>

MeteoConfig ActiveConfig;

MeteoConfig loadConfig()
{
	DynamicJsonDocument doc(1024);
	MeteoConfig cfg;
	if(Json::loadFromFile(doc, METEO_CONFIG_FILE)) {
		JsonObject network = doc["network"];
		cfg.NetworkSSID = network["ssid"].as<const char*>();
		cfg.NetworkPassword = network["password"].as<const char*>();

		JsonObject correction = doc["correction"];
		cfg.AddT = correction["T"];
		cfg.AddRH = correction["RH"];
		cfg.AddTZ = correction["TZ"];

		JsonObject trigger = doc["trigger"];
		cfg.Trigger = TriggerType(trigger["type"].as<int>());
		cfg.RangeMin = trigger["min"];
		cfg.RangeMax = trigger["max"];
	} else {
		cfg.NetworkSSID = WIFI_SSID;
		cfg.NetworkPassword = WIFI_PWD;
	}
	return cfg;
}

void saveConfig(MeteoConfig& cfg)
{
	ActiveConfig = cfg;

	DynamicJsonDocument doc(1024);
	auto network = doc.createNestedObject("network");
	network["ssid"] = cfg.NetworkSSID;
	network["password"] = cfg.NetworkPassword;

	auto correction = doc.createNestedObject("correction");
	correction["T"] = cfg.AddT;
	correction["RH"] = cfg.AddRH;
	correction["TZ"] = cfg.AddTZ;

	auto trigger = doc.createNestedObject("trigger");
	trigger["type"] = (int)cfg.Trigger;
	trigger["min"] = cfg.RangeMin;
	trigger["max"] = cfg.RangeMax;

	Json::saveToFile(doc, METEO_CONFIG_FILE, Json::Pretty);
}
