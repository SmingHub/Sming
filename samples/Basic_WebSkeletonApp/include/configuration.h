#pragma once

#include <WString.h>

const char THERM_CONFIG_FILE[] = ".therm.conf"; // leading point for security reasons :)

// Application configuration JsonBuffer size, increase it if you have large config
const uint8_t ConfigJsonBufferSize = 200;

struct ThermConfig {
	String StaSSID;
	String StaPassword;
	bool StaEnable{true}; // Enable WIFI Client
};

ThermConfig loadConfig();
void saveConfig(ThermConfig& cfg);

extern ThermConfig activeConfig;
