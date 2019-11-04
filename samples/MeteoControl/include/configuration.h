#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <SmingCore.h>
#include <JsonObjectStream.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

// Pin for communication with DHT sensor
//#define DHT_PIN 1 // UART0 TX pin
#define DHT_PIN 12

// Pin for trigger control output
//#define CONTROL_PIN 3 // UART0 RX pin
#define CONTROL_PIN 15

#define METEO_CONFIG_FILE ".meteo.conf" // leading point for security reasons :)

enum TriggerType { eTT_None = 0, eTT_Temperature, eTT_Humidity };

struct MeteoConfig {
	MeteoConfig()
	{
		AddT = 0;
		AddRH = 0;
		AddTZ = 0;
		Trigger = eTT_Temperature;
		RangeMin = 18;
		RangeMax = 29;
	}

	String NetworkSSID;
	String NetworkPassword;

	float AddT;  // Temperature adjustment
	float AddRH; // Humidity adjustment
	float AddTZ; // TimeZone - local time offset

	TriggerType Trigger; // Sensor trigger type
	float RangeMin;
	float RangeMax;
};

MeteoConfig loadConfig();
void saveConfig(MeteoConfig& cfg);
extern void startWebClock();

extern MeteoConfig ActiveConfig;

#endif /* INCLUDE_CONFIGURATION_H_ */
