/*
 * AppSettings.h
 *
 *  Created on: 13 мая 2015 г.
 *      Author: Anakod
 */

#include <SmingCore/SmingCore.h>

#ifndef INCLUDE_APPSETTINGS_H_
#define INCLUDE_APPSETTINGS_H_

#define APP_SETTINGS_FILE ".settings.conf" // leading point for security reasons :)
#define DEF_MEMBERS_DOMAIN	"www.conventus.dk"
struct ApplicationSettingsStorage
{
	String ssid;
	String password;
	bool dhcp = true;
	IPAddress ip;
	IPAddress netmask;
	IPAddress gateway;

	String lockid="0";
	String orgid;
	String orgkey;
	String groups;
	String membersdomain=DEF_MEMBERS_DOMAIN;
	void load()
	{
		DynamicJsonBuffer jsonBuffer;
		if (exist())
		{
			int size = fileGetSize(APP_SETTINGS_FILE);
			char* jsonString = new char[size + 1];
			fileGetContent(APP_SETTINGS_FILE, jsonString, size + 1);
			JsonObject& root = jsonBuffer.parseObject(jsonString);

			JsonObject& network = root["network"];
			ssid = network["ssid"].asString();
			password = network["password"].asString();

			dhcp = network["dhcp"];

			ip = network["ip"].asString();
			netmask = network["netmask"].asString();
			gateway = network["gateway"].asString();

			lockid=network["lockid"].asString();
			orgid=network["orgid"].asString();
			orgkey=network["orgkey"].asString();
			groups=network["groups"].asString();
			membersdomain=network["membersdomain"].asString();


			delete[] jsonString;
		}else{
			Serial.print("\r\nAppSettings file DOES NOT exist \r\n");

		}
	}

	void save()
	{
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

		JsonObject& network = jsonBuffer.createObject();
		root["network"] = network;
		network["ssid"] = ssid.c_str();
		network["password"] = password.c_str();

		network["dhcp"] = dhcp;

		// Make copy by value for temporary string objects
		network["ip"]= ip.toString();
		network["netmask"]= netmask.toString();
		network["gateway"]= gateway.toString();

		network["lockid"]= lockid;
		network["orgid"]= orgid;
		network["membersdomain"]=membersdomain;


		//TODO: add direct file stream writing
		String rootString;
				root.printTo(rootString);
				fileSetContent(APP_SETTINGS_FILE, rootString);
	}

	bool exist() { return fileExist(APP_SETTINGS_FILE); }
};

static ApplicationSettingsStorage AppSettings;

#endif /* INCLUDE_APPSETTINGS_H_ */
