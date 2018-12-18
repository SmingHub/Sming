#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__

typedef enum{
  NO_TLS = 0,                       // 0: disable SSL/TLS, there must be no certificate verify between MQTT server and ESP8266
  TLS_WITHOUT_AUTHENTICATION = 1,   // 1: enable SSL/TLS, but there is no a certificate verify
  ONE_WAY_ANTHENTICATION = 2,       // 2: enable SSL/TLS, ESP8266 would verify the SSL server certificate at the same time
  TWO_WAY_ANTHENTICATION = 3,       // 3: enable SSL/TLS, ESP8266 would verify the SSL server certificate and SSL server would verify ESP8266 certificate
}TLS_LEVEL;


/*IMPORTANT: the following configuration maybe need modified*/
/***********************************************************************************************************************/
#define CFG_HOLDER    0x00FF55A4    /* Change this value to load default configurations */

/*DEFAULT CONFIGURATIONS*/

#define MQTT_HOST            "192.168.1.100" // the IP address or domain name of your MQTT server or MQTT broker ,such as "mqtt.yourdomain.com"
#define MQTT_PORT            1883    // the listening port of your MQTT server or MQTT broker
#define MQTT_CLIENT_ID        "Device_ID"    // the ID of yourself, any string is OK,client would use this ID register itself to MQTT server
#define MQTT_USER            "Device_Name" // your MQTT login name, if MQTT server allow anonymous login,any string is OK, otherwise, please input valid login name which you had registered
#define MQTT_PASS            "Device_Passwd" // you MQTT login password, same as above
#define STA_SSID "AP_SSID"    // your AP/router SSID to config your device networking
#define STA_PASS "AP_Passwd" // your AP/router password

#define DEFAULT_SECURITY    NO_TLS      // very important: you must config DEFAULT_SECURITY for SSL/TLS

#define CA_CERT_FLASH_ADDRESS 0x77              // CA certificate address in flash to read, 0x77 means address 0x77000
#define CLIENT_CERT_FLASH_ADDRESS 0x78          // client certificate and private key address in flash to read, 0x78 means address 0x78000
/***********************************************************************************************************************/


/*Please Keep the following configuration if you have no very deep understanding of ESP SSL/TLS*/
#define CFG_LOCATION    0x79    /* Please don't change or if you know what you doing */
#define MQTT_BUF_SIZE        1024
#define MQTT_KEEPALIVE        120     /*second*/
#define MQTT_RECONNECT_TIMEOUT     5    /*second*/
#define MQTT_SSL_ENABLE //* Please don't change or if you know what you doing */

#define STA_TYPE AUTH_WPA2_PSK
#define QUEUE_BUFFER_SIZE                 2048

#define PROTOCOL_NAMEv31    /*MQTT version 3.1 compatible with Mosquitto v0.15*/
//PROTOCOL_NAMEv311            /*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

#endif // __MQTT_CONFIG_H__
