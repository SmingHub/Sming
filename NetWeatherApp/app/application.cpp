// NetWeather

#include "configuration.h"
#include <SmingCore/SmingCore.h>
#include <Libraries/LiquidCrystal/LiquidCrystal.h>
#include <math.h>

// CANDIDATE FOR A FSM???


#include "special_chars.h"

LiquidCrystal lcd(4,2,14,12,13,15);

Timer heartBeatTimer;
bool heatBeatState = true;

Timer stagingTimer;
Timer weatherTimer;
Timer secondsUpdaterTimer;

HttpClient httpWeather;

bool readyWeather = false;
bool readyTime = false;


void onGetWeather(HttpClient& client, bool successful);
void heartBeatBlink();
void WiFiConnected();
void WiFiFail();
void waitForTimeWeather();
void showTime();
void secondsUpdater();
void getWeather();
void showWeather();
void onGotTime(NtpClient& client, time_t time);


// Update Time every hour
NtpClient ntpClient("pool.ntp.org", SECS_PER_HOUR, onGotTime);



//JSON only knows double
int weather_Temperature;
int weather_Humidity;
String weather_Description;
DateTime weather_SunRise;
DateTime weather_SunSet;


DynamicJsonBuffer jsonConfigBuffer;
// Declare pointer in order to avoid initializing just yet
JsonObject* cfg= NULL;

void saveConfig(JsonObject& cfg_local);

JsonObject& getConfig(){

	if (fileExist(NETWEATHER_CONFIG_FILE))
	{
		int size = fileGetSize(NETWEATHER_CONFIG_FILE);
		char* jsonString = new char[size + 1];
		fileGetContent(NETWEATHER_CONFIG_FILE, jsonString, size + 1);

		JsonObject& config = jsonConfigBuffer.parseObject(jsonString);

		if (config.success())
		{

			return config;

		}

	}

	JsonObject& config=jsonConfigBuffer.createObject();

	JsonObject& network=jsonConfigBuffer.createObject();
	config["network"]=network;
	network["ssid"] = WIFI_SSID;
	network["password"] = WIFI_PWD;

	JsonObject& weather=jsonConfigBuffer.createObject();
	config["weather"]=weather;
	weather["city_id"]="6094817";
	weather["url"]="http://api.openweathermap.org/data/2.5/weather";
	weather["api_key"]="a5e4c3d55c24560c4327ca0a862897c5";
	weather["units"]="metric";
	weather["refresh_rate"]= 5*SECS_PER_MIN*1000;

	// Eastern Standard Time - EST
	config["time_zone"] = -5.0;

	config["display_time"] = 4*1000;


	saveConfig(config);

	return config;
}


void saveConfig(JsonObject& cfg_local){
	Serial.print("JSON CONFIG: ");
	cfg_local.printTo(Serial);
	Serial.println();


}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files
	// Initialize local reference with our config
	JsonObject& cfg_local=getConfig();
	// point global pointer to our reference
	cfg=&cfg_local;


	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial



	//Change CPU freq. to 160MHZ
	System.setCpuFrequency(eCF_160MHz);
	Serial.print("New CPU frequency is:");
	Serial.println((int)System.getCpuFrequency());

	Serial.println("Initializing LCD interface");

	pinMode(HEARTBEAT_LED_PIN, OUTPUT);
	heartBeatTimer.initializeMs(1000, heartBeatBlink).start();

	WifiAccessPoint.enable(false);
	WifiStation.config( cfg_local["network"]["ssid"].toString() , cfg_local["network"]["password"].toString());
	WifiStation.enable(true);


	// set timezone hourly difference to UTC
	SystemClock.setTimeZone(cfg_local["time_zone"].as<double>());


	lcd.begin(16, 2);   // initialize the lcd for 16 chars 2 lines, turn on backlight
	lcd.createChar(1, icon_termometer);
	lcd.createChar(2, icon_water);
	lcd.createChar(3, celsius);
	lcd.createChar(4, icon_clock);
	lcd.createChar(5, icon_wifi);

//-------- Write characters on the display ------------------
// NOTE: Cursor Position: (CHAR, LINE) start at 0
  lcd.clear();
  lcd.print("Connecting to");
  lcd.setCursor(15,0);
  lcd.print("\5");
  lcd.setCursor(0,1);
  lcd.print("WiFi ...");
  lcd.blink();

  WifiStation.waitConnection(WiFiConnected, 30, WiFiFail);



}

void onGotTime(NtpClient &client, time_t time){

	readyTime=true;
	client.setAutoQuery(true);
	client.setAutoUpdateSystemClock(true);
	SystemClock.setTime(time,eTZ_UTC);
}

void WiFiConnected(){
	// Localize the config
	JsonObject& cfg_local = *cfg;

	lcd.clear();
	lcd.print("SSID:");
	lcd.print(WifiStation.getSSID());

	lcd.setCursor(0,1);
	lcd.print("IP:");
	lcd.print(WifiStation.getIP());
	lcd.noBlink();



	getWeather();
	weatherTimer.initializeMs(cfg_local["weather"]["refresh_rate"], getWeather).start();

	stagingTimer.initializeMs(cfg_local["display_time"].as<int>(), waitForTimeWeather).startOnce();

}

void WiFiFail(){
	lcd.clear();
	lcd.print("Still connecting");
	lcd.setCursor(0,1);
	lcd.print("to WiFi ...");
	lcd.blink();

	WifiStation.waitConnection(WiFiConnected, 10, WiFiFail); // Repeat and check again
}

void waitForTimeWeather(){
	// Localize the config
	JsonObject& cfg_local = *cfg;
	lcd.clear();
	lcd.print("Updating Net");
	lcd.setCursor(0,1);
	lcd.print("Time & Weather...");
	lcd.blink();

	// Wait until we have ready weather and time
	if (readyWeather && readyTime ){
		showTime();
	} else {
		Serial.println("WeatherTime NOT READY !!!");
		stagingTimer.initializeMs(500, waitForTimeWeather).startOnce();
	}
}

void showTime(){
	// Localize the config
	JsonObject& cfg_local = *cfg;
	lcd.clear();
	lcd.noBlink();
	lcd.setCursor(2,0);
	lcd.print("\4 ");

	DateTime time_now = SystemClock.now();

	lcd.print(time_now.toShortTimeString(true));
	// Wait 3 seconds then show the weather
	lcd.setCursor(0,1);
	lcd.print("R ");
	lcd.print(weather_SunRise.toShortTimeString());
	lcd.print(" S ");
	lcd.print(weather_SunSet.toShortTimeString());

	// Update the time every 1s so it looks like it's moving and repeat
	secondsUpdaterTimer.initializeMs(1000, secondsUpdater).start(true);

	stagingTimer.initializeMs(cfg_local["display_time"].as<int>(), showWeather).startOnce();
}

void secondsUpdater(){

	lcd.setCursor(4,0);
	DateTime time_now = SystemClock.now();
	lcd.print(time_now.toShortTimeString(true));

}

void showWeather(){
	// Localize the config
	JsonObject& cfg_local = *cfg;
	secondsUpdaterTimer.stop();
	lcd.clear();
	lcd.noBlink();
	lcd.print("\1 ");
	lcd.print(weather_Temperature);
	lcd.print("\3C");
	lcd.setCursor(10,0);
	lcd.print("\2 ");
	lcd.print(weather_Humidity);
	lcd.print("%");
	lcd.setCursor(0,1);
	lcd.print(weather_Description);


	stagingTimer.initializeMs(cfg_local["display_time"].as<int>(), showTime).startOnce();
}

void onGetWeather(HttpClient& client, bool successful) {
	// Localize the config
	JsonObject& cfg_local = *cfg;

//	if (successful){
//		Serial.println("Successful onGetWeather");
//	  // Get weather every 5 minutes
//	  weatherTimer.initializeMs(5*60*1000, getWeather).start();
//
//	}
//	else{
//		Serial.println("Failed onGetWeather");
//		// Get weather every 30s
//		weatherTimer.initializeMs(30*1000, getWeather).start();
//	}

	String response = client.getResponseString();
	Serial.println("Server response: '" + response + "'");
	if (response.length() > 0)
	{
		char *response_char = new char[response.length() + 1];
		strcpy(response_char, response.c_str());
		response = "";

		DynamicJsonBuffer jsonBuffer;
		// apparently "ArduinoJson v5.0-beta, you can avoid the String copy because JsonBuffer::parseObject() can take a String as a parameter"
		JsonObject& jsonWeather = jsonBuffer.parseObject(response_char);
		if (!jsonWeather.success()) {                            // everything OK
		      Serial.println("JSON Parser failed !!!");
		      return;
		  }

		if (jsonWeather["main"]["temp"].is<double>()) {
			// we have a double
			double temp_temp = jsonWeather["main"]["temp"];
			weather_Temperature = round(temp_temp);
		} else {
			// most likely an integer
			weather_Temperature = jsonWeather["main"]["temp"];
		}



		weather_Humidity = (int) jsonWeather["main"]["humidity"];
		weather_Description = jsonWeather["weather"][0]["description"];

		// Adjust for timezone
		weather_SunRise = (long) jsonWeather["sys"]["sunrise"] + (SECS_PER_HOUR * cfg_local["time_zone"].as<double>());
		weather_SunSet = (long) jsonWeather["sys"]["sunset"] + (SECS_PER_HOUR * cfg_local["time_zone"].as<double>());

		Serial.println("===========");
		Serial.print("Temperature: ");
		Serial.println(weather_Temperature);
		Serial.print("Humidity: ");
		Serial.println(weather_Humidity);
		Serial.print("Weather Description: ");
		Serial.println(weather_Description);
		Serial.print("Sunrise Time: ");
		Serial.println(weather_SunRise);
		Serial.print("Sunset Time: ");
		Serial.println(weather_SunSet);
		Serial.println("===========");
		Serial.print("HEAP Free size: ");
	    Serial.println(system_get_free_heap_size());
		Serial.println("===========");

		// Clean up the response_char we just used and the JSON Buffer we just used so we don't fill it up
		delete [] response_char;
		// Jsonbuffer distructor will take care of that

		readyWeather = true;
	}
}



void getWeather(){


	// Localize the config
	JsonObject& cfg_local = *cfg;
	// Extract only weather related config
	JsonObject& cfg_weather = cfg_local["weather"];

	String url = cfg_weather["url"].toString() + "?id=" + cfg_weather["city_id"].toString() + "&units=" + cfg_weather["units"].toString() + "&appid=" + cfg_weather["api_key"].toString();
//	String url = "http://www.quanttrom.com/weather.json";

	httpWeather.downloadString( url ,onGetWeather);

	Serial.println("===========");
	Serial.println("WIFI Status:");
	Serial.println(WifiStation.getConnectionStatusName());
	Serial.println("===========");
}



void heartBeatBlink()
{

	digitalWrite(HEARTBEAT_LED_PIN, heatBeatState);
	heatBeatState = !heatBeatState;

}
