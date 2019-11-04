#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <SmingCore.h>

const char THERM_CONFIG_FILE[] = ".therm.conf"; // leading point for security reasons :)

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

extern ThermConfig ActiveConfig;

#endif /* INCLUDE_CONFIGURATION_H_ */
