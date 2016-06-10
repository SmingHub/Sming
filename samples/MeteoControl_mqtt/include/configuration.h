#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <user_config.h>
#include <SmingCore/SmingCore.h>

//////////////////////////// Wi-Fi config ///////////////////////////////////////
#define WIFI_SSID	"Garage12"
#define WIFI_PWD	"bunkerwifi"

//////////////////////////// MQTT config ///////////////////////////////////////
#define SERVER		"192.168.2.138"
#define CLIENT		"ESP8266_test"
#define LOG		""
#define PASS		""
#define SUB_TOPIC	"testing/#"
#define CONTR_TOPIC	"testing/CONTROL/GPIO/"
#define STAT_TOPIC	"testing/status/GPIO/"

//////////////////////////// IPAddress ESP ///////////////////////////////////////
long ESP_IP = IPAddress(192, 168, 55, 200); // прописываем по желанию

//////////////////////////// Sensors config ///////////////////////////////////////
#define BMP_T		"testing/status/BMP180/Temperature"
#define BMP_P		"testing/status/BMP180/Pressure"
#define SI_T		"testing/status/SI7021/Temperature"
#define SI_H		"testing/status/SI7021/Humidity"
#define VER_TOPIC	"testing/firmware/version"


int TIMER = 20; // every N* seconds send to mqtt server

// Forward declarations
void startMqttClient();
void onMessageReceived(String topic, String message);

// MQTT client
MqttClient mqtt(SERVER, 1883, onMessageReceived);

enum TriggerType
{
	eTT_None = 0,
	eTT_Temperature,
	eTT_Humidity
};


#endif /* INCLUDE_CONFIGURATION_H_ */
