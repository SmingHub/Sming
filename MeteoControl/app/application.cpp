#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/LiquidCrystal/LiquidCrystal_I2C.h>
#include <Libraries/DHT/DHT.h>

///////////////////////////////////////////////////////////////////
// Set your SSID & Pass for initial configuration
#include "../include/configuration.h" // application configuration
///////////////////////////////////////////////////////////////////

#include "special_chars.h"
#include "webserver.h"

DHT dht(DHT_PIN, DHT11);

// For more information visit useful wiki page: http://arduino-info.wikispaces.com/LCD-Blue-I2C
// Standard I2C bus pins: GPIO0 -> SCL, GPIO2 -> SDA
#define I2C_LCD_ADDR 0x27
LiquidCrystal_I2C lcd(I2C_LCD_ADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

Timer procTimer;
Timer displayTimer;
bool state = true;
// Sensors string values
String StrT, StrRH, StrTime;

void process();
void connectOk();
void connectFail();

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial

	ActiveConfig = loadConfig();

	// Select control line
	pinMode(CONTROL_PIN, OUTPUT);

	// DHT sensor start
	dht.begin();

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

	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start

	procTimer.initializeMs(5000, process).start();
	process();
}

void showValues()
{
	lcd.setCursor(0,0);
	// Output time, if it was loaded from remote server
	if (StrTime.length() > 0)
	{
		lcd.print(StrTime);
		lcd.setCursor(0,1);
		lcd.print("  ");
	}
	lcd.print("\1 ");

	lcd.print(StrT);
	lcd.print("\3C ");

	if (StrTime.length() == 0)
	{
		lcd.print("                 "); // Clear line end
		lcd.setCursor(0,1);
	}
	lcd.print("\2 ");
	lcd.print(StrRH);
	lcd.print("%                 ");  // Clear line end
}

void process()
{
	float t = dht.readTemperature() + ActiveConfig.AddT;
	float h = dht.readHumidity() + ActiveConfig.AddRH;

	if (ActiveConfig.Trigger == eTT_Temperature)
		state = t < ActiveConfig.RangeMin || t > ActiveConfig.RangeMax;
	else if (ActiveConfig.Trigger == eTT_Humidity)
		state = h < ActiveConfig.RangeMin || h > ActiveConfig.RangeMax;

	digitalWrite(CONTROL_PIN, state);
	StrT = String(t, 0);
	StrRH = String(h, 0);

	if (!displayTimer.isStarted())
		displayTimer.initializeMs(1000, showValues).start();
}

void connectOk()
{
	debugf("connected");
	WifiAccessPoint.enable(false);
	lcd.clear();
	lcd.print("\7 ");
	lcd.print(WifiStation.getIP().toString());
	// Restart main screen output
	procTimer.restart();
	displayTimer.stop();

	startWebClock();
	// At first run we will download web server content
	if (!fileExist("index.html") || !fileExist("config.html") || !fileExist("api.html") || !fileExist("bootstrap.css.gz") || !fileExist("jquery.js.gz"))
		downloadContentFiles();
	else
		startWebServer();
}

void connectFail()
{
	debugf("connection FAILED");
	WifiAccessPoint.config("MeteoConfig", "", AUTH_OPEN);
	WifiAccessPoint.enable(true);
	// Stop main screen output
	procTimer.stop();
	displayTimer.stop();
	lcd.clear();

	lcd.setCursor(0,0);
	lcd.print("WiFi MeteoConfig");
	lcd.setCursor(0,1);
	lcd.print("  ");
	lcd.print(WifiAccessPoint.getIP());

	startWebServer();
	WifiStation.waitConnection(connectOk); // Wait connection
}

////// WEB Clock //////
Timer clockRefresher;
HttpClient clockWebClient;
uint32_t lastClockUpdate = 0;
DateTime clockValue;
const int clockUpdateIntervalMs = 10 * 60 * 1000; // Update web clock every 10 minutes

void onClockUpdating(HttpClient& client, bool successful)
{
	if (!successful)
	{
		debugf("CLOCK UPDATE FAILED %d (code: %d)", successful, client.getResponseCode());
		lastClockUpdate = 0;
		return;
	}

	// Extract date header from response
	clockValue = client.getServerDate();
	if (clockValue.isNull()) clockValue = client.getLastModifiedDate();
	if (!clockValue.isNull())
		clockValue.addMilliseconds(ActiveConfig.AddTZ * 1000 * 60 * 60);
}

void refreshClockTime()
{
	uint32_t nowClock = millis();
	if (nowClock < lastClockUpdate) lastClockUpdate = 0; // Prevent overflow, restart
	if ((lastClockUpdate == 0 || nowClock - lastClockUpdate > clockUpdateIntervalMs) && !clockWebClient.isProcessing())
	{
		clockWebClient.downloadString("google.com", onClockUpdating);
		lastClockUpdate = nowClock;
	}
	else if (!clockValue.isNull())
		clockValue.addMilliseconds(clockRefresher.getIntervalMs());

	if (!clockValue.isNull())
	{
		StrTime = clockValue.toShortDateString() + " " + clockValue.toShortTimeString(false);

		if ((millis() % 2000) > 1000)
			StrTime.setCharAt(13, ' ');
		else
			StrTime.setCharAt(13, ':');
	}
}

void startWebClock()
{
	lastClockUpdate = 0;
	clockRefresher.stop();
	clockRefresher.initializeMs(500, refreshClockTime).start();
}
