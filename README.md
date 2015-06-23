# Sming
Sming - Open Source framework for high efficiency WiFi SoC ESP8266 native development with C++ language.

[![ESP8266 C++ development framework](https://github.com/anakod/Sming/wiki/images/small/combine.png)](https://github.com/anakod/Sming/wiki/examples)

[![Join the chat at https://gitter.im/alonewolfx2/Sming](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/alonewolfx2/Sming?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Donate](http://img.shields.io/paypal/donate.png?color=yellow)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=WAQ8XDHCKU3PL&lc=US&item_name=Sming%20Framework%20development&item_number=sming&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)
[![Download](https://img.shields.io/badge/download-~1.7M-orange.svg)](https://github.com/anakod/Sming/releases/latest)


## Summary
* Fast & user friendly development
* Work with GPIO in Arduino style
* High effective in perfomance and memory usage (this is native firmware!)
* Compatible with standard Arduino libraries - use any popular hardware in few lines of code
* OTA (cloud) firmware updating
* Build-in file system: [spiffs](https://github.com/pellepl/spiffs)
* Build-in powerfull network and wireless modules
* Build-in great JSON library: [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* HTTP, AJAX, WebSockets support
* MQTT protocol based on [libemqtt] (https://github.com/menudoproblema/libemqtt)
* Open source [LWIP](https://github.com/kadamski/esp-lwip) stack
* Simple and powerfull hardware API wrappers
* Based on Espressif SDK v1.0.1b1

## Getting started
- [Windows](https://github.com/anakod/Sming/wiki/Windows-Quickstart)
- [Linux](https://github.com/anakod/Sming/wiki/Linux-Quickstart)
- [MacOS](https://github.com/anakod/Sming/wiki/MacOS-Quickstart)

You can find more information about compilation and flashing process by reading esp8266.com forum discussion thread.

## Examples
More information at **[Wiki Examples](https://github.com/anakod/Sming/wiki/examples)** page.

### Simple GPIO input/output
```c++
#define LED_PIN 2 // GPIO2
...
pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, HIGH);
```

### Connect to WiFi and start Serial communication
```c++
Serial.begin(9600);
Serial.println("Hello Sming! Let's do smart things.");

WifiStation.enable(true);
WifiStation.config("LOCAL-NETWORK", "123456789087"); // Put you SSID and Password here
```

### Read DHT22 sensor
```c++
#include <Libraries/DHT/DHT.h> // This is just popular Arduino library!

#define WORK_PIN 0 // GPIO0
DHT dht(WORK_PIN, DHT22);

void init()
{
	dht.begin();

	float h = dht.readHumidity();
	float t = dht.readTemperature();
}
```

### HTTP client
```c++
HttpClient thingSpeak;
...
thingSpeak.downloadString("http://api.thingspeak.com/update?key=XXXXXXX&field1=" + String(sensorValue), onDataSent);

void onDataSent(HttpClient& client, bool successful)
{
	if (successful)
		Serial.println("Successful!");
	else
		Serial.println("Failed");
}
```

### Embedded HTTP WebServer
```c++
server.listen(80);
server.addPath("/", onIndex);
server.addPath("/hello", onHello);
server.setDefaultHandler(onFile);

Serial.println("=== WEB SERVER STARTED ===");
Serial.println(WifiStation.getIP());

...

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	vars["counter"] = String(counter);
	vars["IP"] = WifiStation.getIP().toString();
	vars["MAC"] = WifiStation.getMAC();
	response.sendTemplate(tmpl);
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);
		
	response.setCache(86400, true);
	response.sendFile(file);
}
```
