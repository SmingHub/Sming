#include <SmingCore.h>
#include <Hosted/Client.h>

Hosted::Client* hostedClient{nullptr};

#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

extern void init();

void host_init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	hostedClient = new Hosted::Client(Serial);
	init();
}
