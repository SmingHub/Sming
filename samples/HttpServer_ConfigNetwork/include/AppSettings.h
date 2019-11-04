/*
 * AppSettings.h
 *
 *  Created on: 13 ??? 2015 ?.
 *      Author: Anakod
 */

#include <SmingCore.h>
#include <ArduinoJson.h>

#ifndef INCLUDE_APPSETTINGS_H_
#define INCLUDE_APPSETTINGS_H_

#define APP_SETTINGS_FILE ".settings.conf" // leading point for security reasons :)

struct ApplicationSettingsStorage {
	String ssid;
	String password;

	bool dhcp = true;

	IpAddress ip;
	IpAddress netmask;
	IpAddress gateway;

	void load()
	{
		DynamicJsonDocument doc(1024);
		if(Json::loadFromFile(doc, APP_SETTINGS_FILE)) {
			JsonObject network = doc["network"];
			ssid = network["ssid"].as<const char*>();
			password = network["password"].as<const char*>();

			dhcp = network["dhcp"] | false;

			ip = network["ip"].as<const char*>();
			netmask = network["netmask"].as<const char*>();
			gateway = network["gateway"].as<const char*>();
		}
	}

	void save()
	{
		DynamicJsonDocument doc(1024);

		auto network = doc.createNestedObject("network");
		network["ssid"] = ssid.c_str();
		network["password"] = password.c_str();

		network["dhcp"] = dhcp;

		network["ip"] = ip.toString();
		network["netmask"] = netmask.toString();
		network["gateway"] = gateway.toString();

		Json::saveToFile(doc, APP_SETTINGS_FILE);
	}

	bool exist()
	{
		return fileExist(APP_SETTINGS_FILE);
	}
};

static ApplicationSettingsStorage AppSettings;

#endif /* INCLUDE_APPSETTINGS_H_ */
