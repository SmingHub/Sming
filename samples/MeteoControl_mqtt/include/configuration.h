#pragma once

#include <Network/MqttClient.h>

//////////////////////////// Wi-Fi config ///////////////////////////////////////
#ifndef WIFI_SSID
#define WIFI_SSID "Wifi SSID"
#define WIFI_PWD "Wifi password"
#endif

//////////////////////////// MQTT config ///////////////////////////////////////
#ifndef MQTT_SERVER
#define MQTT_SERVER "192.168.2.138"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif
#define CLIENT "ESP8266_test"
#define LOG ""
#define PASS ""

#define SUB_TOPIC "testing/#"
#define CONTR_TOPIC "testing/CONTROL/GPIO/"
#define STAT_TOPIC "testing/status/GPIO/"

//////////////////////////// Sensors config ///////////////////////////////////////
#define BMP_T "testing/status/BMP180/Temperature"
#define BMP_P "testing/status/BMP180/Pressure"
#define SI_T "testing/status/SI7021/Temperature"
#define SI_H "testing/status/SI7021/Humidity"
#define VER_TOPIC "testing/firmware/version"

constexpr int TIMER = 20; // every N* seconds send to mqtt server

enum TriggerType {
	eTT_None = 0,
	eTT_Temperature,
	eTT_Humidity,
};

void startMqttClient();

extern MqttClient mqtt;
