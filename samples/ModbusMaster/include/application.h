#ifndef APPLICATION_H
#define APPLICATION_H

#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

struct WiFiConfig {
	String SSID;
	String Pass;
};

#define RELAY_PIN 4
#define OPTO_PIN 5

void mbLoop(void);
void preTransmission(void);
void postTransmission(void);
void mbLogReceive(const uint8_t*, size_t, uint8_t);
void mbLogTransmit(const uint8_t*, size_t);

#endif /* APPLICATION_H */
