#include <application.h>
#include <user_config.h>

ApplicationClass App;

void init()
{
	App.init();
	App.start();
}

void ApplicationClass::init()
{
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false);
	Serial.commandProcessing(true);

	_initialWifiConfig();

	Config.load();

	// Attach Wifi events handlers
	WifiEvents.onStationDisconnect(onStationDisconnectDelegate(&ApplicationClass::_STADisconnect, this));
	WifiEvents.onStationAuthModeChange(onStationAuthModeChangeDelegate(&ApplicationClass::_STAAuthModeChange, this));
	WifiEvents.onStationGotIP(onStationGotIPDelegate(&ApplicationClass::_STAGotIP, this));

	startWebServer();
}

void ApplicationClass::start()
{
	_loopTimer.initializeMs(Config.loopInterval, TimerDelegate(&ApplicationClass::_loop, this)).start(true);
	_loop();
}

void ApplicationClass::stop()
{
	_loopTimer.stop();
}

void ApplicationClass::_loop()
{
	_counter++;
}

void ApplicationClass::_initialWifiConfig()
{
// Set DHCP hostname to WebAppXXXX where XXXX is last 4 digits of MAC address
	String macDigits =  WifiStation.getMAC().substring(8,12);
	macDigits.toUpperCase();
	WifiStation.setHostname("WebApp" + macDigits);

// One-time set own soft Access Point SSID and PASSWORD and save it into configuration area
// This part of code will run ONCE after application flash into the ESP
	if(WifiAccessPoint.getSSID() != WIFIAP_SSID)
	{
		WifiAccessPoint.config(WIFIAP_SSID, WIFIAP_PWD, AUTH_WPA2_PSK);
		WifiAccessPoint.enable(true, true);
	}
	else
		Serial.printf("AccessPoint already configured.\n");

// One-time set initial SSID and PASSWORD for Station mode and save it into configuration area
// This part of code will run ONCE after application flash into the ESP if there is no
// pre-configured SSID and PASSWORD found in configuration area. Later you can change
// Station SSID and PASSWORD from Web UI and they will NOT overwrite by this part of code

	if (WifiStation.getSSID().length() == 0)
	{
		WifiStation.config(WIFI_SSID, WIFI_PWD);
		WifiStation.enable(true, true);
		WifiAccessPoint.enable(false, true);
	}
	else
		Serial.printf("Station already configured.\n");
}

void ApplicationClass::_STADisconnect(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason)
{
	debugf("DISCONNECT - SSID: %s, REASON: %d\n", ssid.c_str(), reason);

	if (!WifiAccessPoint.isEnabled())
	{
		debugf("Starting OWN AP");
		WifiStation.disconnect();
		WifiAccessPoint.enable(true);
		WifiStation.connect();
	}
}

void ApplicationClass::_STAAuthModeChange(uint8_t oldMode, uint8_t newMode)
{
	debugf("AUTH MODE CHANGE - OLD MODE: %d, NEW MODE: %d\n", oldMode, newMode);

	if (!WifiAccessPoint.isEnabled())
	{
		debugf("Starting OWN AP");
		WifiStation.disconnect();
		WifiAccessPoint.enable(true);
		WifiStation.connect();
	}
}

void ApplicationClass::_STAGotIP(IPAddress ip, IPAddress mask, IPAddress gateway)
{
	debugf("GOTIP - IP: %s, MASK: %s, GW: %s\n", ip.toString().c_str(),
																mask.toString().c_str(),
																gateway.toString().c_str());

	if (WifiAccessPoint.isEnabled())
	{
		debugf("Shutdown OWN AP");
		WifiAccessPoint.enable(false);
	}
	// Add commands to be executed after successfully connecting to AP and got IP from it
}

void ApplicationClass::startWebServer()
{
	if (_webServerStarted) return;

	webServer.listen(80);
	webServer.addPath("/",HttpPathDelegate(&ApplicationClass::_httpOnIndex,this));
	webServer.addPath("/config",HttpPathDelegate(&ApplicationClass::_httpOnConfiguration,this));
	webServer.addPath("/config.json",HttpPathDelegate(&ApplicationClass::_httpOnConfigurationJson,this));
	webServer.addPath("/state.json",HttpPathDelegate(&ApplicationClass::_httpOnStateJson,this));
	webServer.setDefaultHandler(HttpPathDelegate(&ApplicationClass::_httpOnFile,this));
	_webServerStarted = true;

	if (WifiStation.isEnabled())
		debugf("STA: %s", WifiStation.getIP().toString().c_str());
	if (WifiAccessPoint.isEnabled())
		debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}

void ApplicationClass::_httpOnFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void ApplicationClass::_httpOnIndex(HttpRequest &request, HttpResponse &response)
{
	response.setCache(86400, true); // It's important to use cache for better performance.
	response.sendFile("index.html");
}

void ApplicationClass::_httpOnStateJson(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["counter"] = _counter;

	response.sendJsonObject(stream);
}

void ApplicationClass::_httpOnConfiguration(HttpRequest &request, HttpResponse &response)
{

	if (request.getRequestMethod() == RequestMethod::POST)
	{
		debugf("Update configuration\n");

		if (request.getBody() == NULL)
		{
			debugf("Empty Request Body!\n");
			return;
		}
		else // Request Body Not Empty
		{
//Uncomment next line for extra debuginfo
//			Serial.printf(request.getBody());
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
//Uncomment next line for extra debuginfo
//			root.prettyPrintTo(Serial);

			//Mandatory part to setup WIFI
			_handleWifiConfig(root);

			//Application config processing

			if (root["loopInterval"].success()) // There is loopInterval parameter in json
			{
				Config.loopInterval = root["loopInterval"];
				start(); // restart main application loop with new loopInterval setting
			}
			Config.save();
		} // Request Body Not Empty
	} // Request method is POST
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile("config.html");
	}
}

void ApplicationClass::_handleWifiConfig(JsonObject& root)
{
	String StaSSID = root["StaSSID"].success() ? String((const char *)root["StaSSID"]) : "";
	String StaPassword = root["StaPassword"].success() ? String((const char *)root["StaPassword"]) : "";
	uint8_t StaEnable = root["StaEnable"].success() ? root["StaEnable"] : 255;

	if (StaEnable != 255) // WiFi Settings
	{
		if (StaEnable)
		{
			if (WifiStation.isEnabled())
			{
				WifiAccessPoint.enable(false);
			}
			else
			{
				WifiStation.enable(true, true);
				WifiAccessPoint.enable(false, true);
			}
			if (WifiStation.getSSID() != StaSSID || (WifiStation.getPassword() != StaPassword && StaPassword.length() >= 8))
			{
				WifiStation.config(StaSSID, StaPassword);
				WifiStation.connect();
			}
		}
		else
		{
				WifiStation.enable(false, true);
				WifiAccessPoint.enable(true, true);
				WifiAccessPoint.config(WIFIAP_SSID, WIFIAP_PWD, AUTH_WPA2_PSK);
		}
	} //Wifi settings
}

void ApplicationClass::_httpOnConfigurationJson(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	//Mandatory part of WIFI Station SSID & Station mode enable config
	json["StaSSID"] = WifiStation.getSSID();
	json["StaEnable"] = WifiStation.isEnabled() ? 1 : 0;

	//Application configuration parameters
	json["loopInterval"] = Config.loopInterval;

	response.sendJsonObject(stream);
}

void ApplicationConfig::load()
{
	DynamicJsonBuffer jsonBuffer;

	if (fileExist(_fileName))
	{
		int size = fileGetSize(_fileName);
		char* jsonString = new char[size + 1];
		fileGetContent(_fileName, jsonString, size + 1);

		JsonObject& root = jsonBuffer.parseObject(jsonString);

		loopInterval = root["loopInterval"];

		delete[] jsonString;
	}
	else
	{
		//Factory defaults if no config file present
		loopInterval = 1000; // 1 second
	}
}

void ApplicationConfig::save()
{
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["loopInterval"] = loopInterval;

	String buf;
	root.printTo(buf);
	fileSetContent(_fileName, buf);
}
