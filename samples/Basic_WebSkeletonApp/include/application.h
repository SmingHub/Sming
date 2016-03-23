#ifndef INCLUDE_APPLICATION_H_
#define INCLUDE_APPLICATION_H_
#include <SmingCore/SmingCore.h>

// If you want, you can define settings globally in Operation System ENV
// or better in Eclipse Environment Variables

//Wifi Station mode SSID and PASSWORD
#ifndef WIFI_SSID
        #define WIFI_SSID "PleaseEnterSSID" // Put you Station mode SSID and Password here
        #define WIFI_PWD "PleaseEnterPass"
#endif
//OWN Access Point mode SSID and PASSWORD
#ifndef WIFIAP_SSID
        #define WIFIAP_SSID "WebApp" // Put you Station mode SSID and Password here
        #define WIFIAP_PWD "smingweb"
#endif
//Main application config file name
#ifndef APP_CONFIG_FILE
	#define APP_CONFIG_FILE ".app.conf" // leading point for security reasons :)
#endif

class ApplicationConfig
{
public:
	void load();
	void save();
	// Configuration parameters goes next
	uint32_t loopInterval = 0; // loop interval in ms
private:
	String _fileName = APP_CONFIG_FILE;
};

class ApplicationClass
{
public:
	void init(); // Application initialization
	void start(); // Application main-loop start/restart
	void stop(); // Application main-loop stop
	ApplicationConfig Config; // Instance of Configuration for application
	HttpServer webServer; // instance of web server for application
	void startWebServer(); // Start Application WebServer
protected:
	void _loop(); // Application main loop function goes here
	void _initialWifiConfig(); // Doing initial configuration of both Station and AccessPoint
	void _STADisconnect(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason); // Runs when Station disconnects
	void _STAAuthModeChange(uint8_t oldMode, uint8_t newMode); // Runs when Station auth mode changes
	void _STAGotIP(IPAddress ip, IPAddress mask, IPAddress gateway); // Runs when Station got ip from access-point
	void _httpOnFile(HttpRequest &request, HttpResponse &response);
	void _httpOnIndex(HttpRequest &request, HttpResponse &response);
	void _httpOnStateJson(HttpRequest &request, HttpResponse &response);
	void _httpOnConfiguration(HttpRequest &request, HttpResponse &response);
	void _httpOnConfigurationJson(HttpRequest &request, HttpResponse &response);
	void _handleWifiConfig(JsonObject& root);
	uint32_t _counter = 0; // Kind of heartbeat counter
//	uint16_t _loopInterval;
	Timer _loopTimer; // Timer for serving loop
	uint8_t _webServerStarted = false;

};
#endif /* INCLUDE_HEATCONTROL_H_ */
