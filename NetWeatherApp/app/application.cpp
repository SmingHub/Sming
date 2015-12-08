// NetWeather


#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/LiquidCrystal/LiquidCrystal.h>
#include <math.h>

#define HEARTBEAT_LED_PIN 16 // GPIO0 16 on ESP8266; red LED

#define MODE_SWITCH_DELAY 4000 // Switch every 4 seconds

#include "special_chars.h"

// For more information visit useful wiki page: http://arduino-info.wikispaces.com/LCD-Blue-I2C
LiquidCrystal lcd(4,2,14,12,13,15);

Timer heartBeatTimer;
bool heatBeatState = true;

Timer stagingTimer;
Timer weatherTimer;
Timer secondsUpdaterTimer;

HttpClient httpWeather;




void onGetWeather(HttpClient& client, bool successful);
void heartBeatBlink();
void WiFiConnected();
void WiFiFail();
void getTime();
void showTime();
void secondsUpdater();
void getWeather();
void showWeather();

// Update time every 60 minutes ; no delegate function
NtpClient ntpClient("pool.ntp.org", 60*60);

//  Don't use Dynamic Json Buffer. Go to static
StaticJsonBuffer<5000>  jsonBuffer;

//JSON only knows double
int weather_Temperature;
int weather_Humidity;
String weather_Description;
DateTime weather_SunRise;
DateTime weather_SunSet;

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	//Change CPU freq. to 160MHZ
	System.setCpuFrequency(eCF_160MHz);
	Serial.print("New CPU frequency is:");
	Serial.println((int)System.getCpuFrequency());

	Serial.println("Initializing LCD interface");

	pinMode(HEARTBEAT_LED_PIN, OUTPUT);
	heartBeatTimer.initializeMs(1000, heartBeatBlink).start();


	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);


	// set timezone hourly difference to UTC
	SystemClock.setTimeZone(-5);


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

  // Get weather every 5 minutes
  weatherTimer.initializeMs(5*60*1000, getWeather).start(true);



}

void WiFiConnected(){

	lcd.clear();
	lcd.print("SSID:");
	lcd.print(WifiStation.getSSID());

	lcd.setCursor(0,1);
	lcd.print("IP:");
	lcd.print(WifiStation.getIP());
	lcd.noBlink();



	getWeather();

	stagingTimer.initializeMs(5000, getTime).start(false);

}

void WiFiFail(){
	lcd.clear();
	lcd.print("Still connecting");
	lcd.setCursor(0,1);
	lcd.print("to WiFi ...");
	lcd.blink();

	WifiStation.waitConnection(WiFiConnected, 10, WiFiFail); // Repeat and check again
}

void getTime(){
	lcd.clear();
	lcd.print("Updating Net");
	lcd.setCursor(0,1);
	lcd.print("Time & Weather...");
	lcd.blink();

	// Wait 5 seconds then show the time
	stagingTimer.initializeMs(5000, showTime).start(false);

}

void showTime(){

	lcd.clear();
	lcd.noBlink();
	lcd.setCursor(2,0);
	lcd.print("\4 ");
	// check if time is good in the future
	lcd.print(SystemClock.getSystemShortTimeString(eTZ_Local,true));
	// Wait 3 seconds then show the weather
	lcd.setCursor(0,1);
	lcd.print("R ");
	lcd.print(weather_SunRise.toShortTimeString());
	lcd.print(" S ");
	lcd.print(weather_SunSet.toShortTimeString());

	// Update the time every 1s so it looks like it's moving and repeat
	secondsUpdaterTimer.initializeMs(1000, secondsUpdater).start(true);

	stagingTimer.initializeMs(MODE_SWITCH_DELAY, showWeather).start(false);
}

void secondsUpdater(){
	lcd.setCursor(4,0);
	lcd.print(SystemClock.getSystemShortTimeString(eTZ_Local,true));

}

void showWeather(){
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

	// Wait 3 seconds then show the time
	stagingTimer.initializeMs(MODE_SWITCH_DELAY, showTime).start(false);
}

void onGetWeather(HttpClient& client, bool successful)
{
	if (successful){
		Serial.println("Successful onGetWeather");
	  // Get weather every 5 minutes
	  weatherTimer.initializeMs(5*60*1000, getWeather).start(true);
//	  weatherTimer.initializeMs(5*1000, getWeather).start(true);
	}
	else{
		Serial.println("Failed onGetWeather");
		// Get weather every 30s
		weatherTimer.initializeMs(30*1000, getWeather).start(true);
	}

	String response = client.getResponseString();
	Serial.println("Server response: '" + response + "'");
	if (response.length() > 0)
	{
		char *response_char = new char[response.length() + 1];
		strcpy(response_char, response.c_str());

		// apparently "ArduinoJson v5.0-beta, you can avoid the String copy because JsonBuffer::parseObject() can take a String as a parameter"
		JsonObject& root = jsonBuffer.parseObject(response_char);
		if (!root.success()) {                            // everything OK
		      Serial.println("JSON Parser failed !!!");
		      return;
		  }

		if (root["main"]["temp"].is<double>()) {
			// we have a double
			double temp_temp = root["main"]["temp"];
			weather_Temperature = round(temp_temp);
		} else {
			// most likely an integer
			weather_Temperature = root["main"]["temp"];
		}



		weather_Humidity = (int) root["main"]["humidity"];
		weather_Description = root["weather"][0]["description"];

		// crude dirty TimeZone Subtract
		weather_SunRise = (long) root["sys"]["sunrise"] - (60*60*5);
		weather_SunSet = (long) root["sys"]["sunset"]- (60*60*5);

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
		jsonBuffer = StaticJsonBuffer<5000>();

	}
}



void getWeather(){

	httpWeather.downloadString("http://api.openweathermap.org/data/2.5/weather?id=6094817&units=metric&appid=a5e4c3d55c24560c4327ca0a862897c5", onGetWeather);
//	httpWeather.downloadString("http://www.quanttrom.com/weather.json", onGetWeather);

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
