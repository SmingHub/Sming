#pragma once

#include <SmingCore.h>

const char THERM_CONFIG_FILE[] = ".therm.conf"; // leading point for security reasons :)

// Application configuration JsonBuffer size, increase it if you have large config
const uint8_t ConfigJsonBufferSize = 200;

struct ThermConfig {
	ThermConfig()
	{
		StaEnable = 1; //Enable WIFI Client
	}

	String StaSSID;
	String StaPassword;
	uint8_t StaEnable;

	// ThermControl settings
};

ThermConfig loadConfig();
void saveConfig(ThermConfig& cfg);

extern ThermConfig activeConfig;
