#include <SmingCore.h>
#include <Libraries/LiquidCrystal/LiquidCrystal_I2C.h>
#include <Libraries/DHTesp/DHTesp.h>

///////////////////////////////////////////////////////////////////
// Set your SSID & Pass for initial configuration
#include "configuration.h" // application configuration
///////////////////////////////////////////////////////////////////

#include "special_chars.h"
#include "webserver.h"

// Sensors string values
String StrT, StrRH, StrTime;

namespace
{
DHTesp dht;

// For more information visit useful wiki page: http://arduino-info.wikispaces.com/LCD-Blue-I2C
// Standard I2C bus pins: GPIO0 -> SCL, GPIO2 -> SDA
#define I2C_LCD_ADDR 0x27
LiquidCrystal_I2C lcd(I2C_LCD_ADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

SimpleTimer procTimer;
SimpleTimer displayTimer;
bool state = true;

void showValues()
{
	lcd.setCursor(0, 0);
	// Output time, if it was loaded from remote server
	if(StrTime.length() > 0) {
		lcd.print(StrTime);
		lcd.setCursor(0, 1);
		lcd.print("  ");
	}
	lcd.print("\1 ");

	lcd.print(StrT);
	lcd.print("\3C ");

	if(StrTime.length() == 0) {
		lcd.print("                 "); // Clear line end
		lcd.setCursor(0, 1);
	}
	lcd.print("\2 ");
	lcd.print(StrRH);
	lcd.print("%                 "); // Clear line end
}

void process()
{
	float t = dht.getTemperature() + ActiveConfig.AddT;
	float h = dht.getHumidity() + ActiveConfig.AddRH;

	if(ActiveConfig.Trigger == eTT_Temperature) {
		state = t < ActiveConfig.RangeMin || t > ActiveConfig.RangeMax;
	} else if(ActiveConfig.Trigger == eTT_Humidity) {
		state = h < ActiveConfig.RangeMin || h > ActiveConfig.RangeMax;
	}

	digitalWrite(CONTROL_PIN, state);
	StrT = String(t, 0);
	StrRH = String(h, 0);

	if(!displayTimer.isStarted()) {
		displayTimer.initializeMs<1000>(showValues).start();
	}
}

void connectOk(const String& SSID, MacAddress bssid, uint8_t channel)
{
	debugf("connected");
	WifiAccessPoint.enable(false);
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	lcd.clear();
	lcd << "\7 " << ip;
	// Restart main screen output
	procTimer.restart();
	displayTimer.stop();

	startWebClock();
	// At first run we will download web server content
	if(!fileExist("index.html") || !fileExist("config.html") || !fileExist("api.html") ||
	   !fileExist("bootstrap.css.gz") || !fileExist("jquery.js.gz")) {
		downloadContentFiles();
	} else {
		startWebServer();
	}
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("connection FAILED: %s", WifiEvents.getDisconnectReasonDesc(reason).c_str());
	WifiAccessPoint.config("MeteoConfig", "", AUTH_OPEN);
	WifiAccessPoint.enable(true);
	// Stop main screen output
	procTimer.stop();
	displayTimer.stop();
	lcd.clear();

	lcd.setCursor(0, 0);
	lcd.print("WiFi MeteoConfig");
	lcd.setCursor(0, 1);
	lcd.print("  ");
	lcd.print(WifiAccessPoint.getIP());

	startWebServer();
	WifiStation.disconnect();
	WifiStation.connect();
}

////// WEB Clock //////

SimpleTimer clockRefresher;
HttpClient clockWebClient;
uint32_t lastClockUpdate;
DateTime clockValue;
const int clockUpdateIntervalMs = 10 * 60 * 1000; // Update web clock every 10 minutes

int onClockUpdating(HttpConnection& client, bool successful)
{
	auto& response = *client.getResponse();

	if(!successful) {
		Serial << _F("CLOCK UPDATE FAILED, ") << response.code << " " << toString(response.code) << endl;
		lastClockUpdate = 0;
		return -1;
	}

	// Extract date header from response
	clockValue = response.headers.getServerDate();
	if(clockValue.isNull()) {
		clockValue = response.headers.getLastModifiedDate();
	}
	if(!clockValue.isNull()) {
		clockValue.addMilliseconds(ActiveConfig.AddTZ * 1000 * 60 * 60);
	}

	return 0;
}

void refreshClockTime()
{
	uint32_t nowClock = millis();
	if(nowClock < lastClockUpdate) {
		lastClockUpdate = 0; // Prevent overflow, restart
	}
	if((lastClockUpdate == 0 || nowClock - lastClockUpdate > clockUpdateIntervalMs)) {
		clockWebClient.downloadString("google.com", onClockUpdating);
		lastClockUpdate = nowClock;
	} else if(!clockValue.isNull()) {
		clockValue.addMilliseconds(clockRefresher.getIntervalMs());
	}

	if(clockValue.isNull()) {
		return;
	}

	StrTime = clockValue.toShortDateString() + " " + clockValue.toShortTimeString(false);
	StrTime.setCharAt(13, ((nowClock % 2000) > 1000) ? ' ' : ':');
}

} // namespace

void startWebClock()
{
	lastClockUpdate = 0;
	clockRefresher.initializeMs<500>(refreshClockTime).start();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);  // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial

	spiffs_mount(); // Mount file system, in order to work with files

	ActiveConfig = loadConfig();

	// Select control line
	pinMode(CONTROL_PIN, OUTPUT);

	// DHT sensor start
	dht.setup(DHT_PIN, DHTesp::DHT11);

	lcd.begin(16, 2);
	lcd.backlight();
	lcd.createChar(1, icon_termometer);
	lcd.createChar(2, icon_water);
	lcd.createChar(3, celsius);
	lcd.createChar(4, icon_retarrow);
	lcd.createChar(5, icon_clock);
	lcd.createChar(6, icon_cross);
	lcd.createChar(7, icon_check);

	WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	WifiEvents.onStationConnect(connectOk);
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);

	procTimer.initializeMs<5000>(process).start();
	process();
}
