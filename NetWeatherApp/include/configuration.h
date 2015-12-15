#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <user_config.h>
#include <SmingCore/SmingCore.h>


#define HEARTBEAT_LED_PIN 16 // GPIO0 16 on ESP8266; red LED
#define NETWEATHER_CONFIG_FILE "netweather.conf" // leading point for security reasons :)


// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif



//class NetWeatherCfg : public DynamicJsonBuffer , public JsonObject (DynamicJsonBuffer.) {
//
//String NetworkSSID;
//String NetworkPassword;
//
//
//
//public:
////JsonObject& cfg ( jsonBuffer.createObject() )
////	JsonObject *cfg;
//	NetWeatherCfg ();
//	String getNetSSID ();
//	String getNetPassword ();
//
//	void saveCfg ();
//};




#endif /* INCLUDE_CONFIGURATION_H_ */
