#include "../include/configuration.h"

#include <SmingCore/SmingCore.h>

MeteoConfig ActiveConfig;

MeteoConfig loadConfig()
{
	DynamicJsonBuffer jsonBuffer;
	MeteoConfig cfg;
	if (fileExist(METEO_CONFIG_FILE))
	{
		int size = fileGetSize(METEO_CONFIG_FILE);
		char* json = new char[size + 1];
		fileGetContent(METEO_CONFIG_FILE, json, size + 1);
		JsonObject& root = jsonBuffer.parseObject(json);

		JsonObject& network = root["network"];
		cfg.NetworkSSID = String((const char*)network["ssid"]);
		cfg.NetworkPassword = String((const char*)network["password"]);

		JsonObject& correction = root["correction"];
		cfg.AddT = correction["T"];
		cfg.AddRH = correction["RH"];
		cfg.AddTZ = correction["TZ"];

		JsonObject& trigger = root["trigger"];
		cfg.Trigger = (TriggerType)(int)trigger["type"];
		cfg.RangeMin = trigger["min"];
		cfg.RangeMax = trigger["max"];

		delete[] json;
	}
	else
	{
		cfg.NetworkSSID = WIFI_SSID;
		cfg.NetworkPassword = WIFI_PWD;
	}
	return cfg;
}

void saveConfig(MeteoConfig& cfg)
{
	ActiveConfig = cfg;

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& network = jsonBuffer.createObject();
	root["network"] = network;
	network["ssid"] = cfg.NetworkSSID.c_str();
	network["password"] = cfg.NetworkPassword.c_str();

	JsonObject& correction = jsonBuffer.createObject();
	root["correction"] = correction;
	correction["T"] = cfg.AddT;
	correction["RH"] = cfg.AddRH;
	correction["TZ"] = cfg.AddTZ;

	JsonObject& trigger = jsonBuffer.createObject();
	root["trigger"] = trigger;
	trigger["type"] = (int)cfg.Trigger;
	trigger["min"] = cfg.RangeMin;
	trigger["max"] = cfg.RangeMax;

	char buf[3048];
	root.prettyPrintTo(buf, sizeof(buf));
	fileSetContent(METEO_CONFIG_FILE, buf);
}


