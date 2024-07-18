#include <SmingCore.h>
#include <JsonObjectStream.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
HttpServer server;
FtpServer ftp;

// Set input GPIO pins here
const uint8_t inputs[] = {5, 2};

void onIndex(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	String gpioList;
	for(unsigned i = 0; i < ARRAY_SIZE(inputs); ++i) {
		String s = F("<span id=\"gpio{id}\" class=\"label label-default\">GPIO{gpio}</span> ");
		s.replace("{id}", String(i));
		s.replace("{gpio}", String(inputs[i]));
		gpioList += s;
	}
	vars["gpio_list"] = gpioList;
	response.sendNamedStream(tmpl); // this template object will be deleted automatically
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();

	if(file[0] == '.')
		response.code = HTTP_STATUS_FORBIDDEN;
	else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onAjaxInput(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();
	json["status"] = (bool)true;

	String stringKey = F("StringKey");
	String stringValue = F("StringValue");

	json[stringKey] = stringValue;

	for(int i = 0; i < 11; i++) {
		String desiredString = F("sensor_") + String(i);
		json[desiredString] = desiredString;
	}

	JsonObject gpio = json.createNestedObject("gpio");
	for(unsigned i = 0; i < ARRAY_SIZE(inputs); ++i) {
		gpio[String(i)] = digitalRead(inputs[i]);
	}

	response.sendDataStream(stream, MIME_JSON);
}

void onAjaxFrequency(HttpRequest& request, HttpResponse& response)
{
	int freq = request.getQueryParameter("value").toInt();
	System.setCpuFrequency(CpuFrequency(freq));

	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();
	json["status"] = true;
	json["value"] = (int)System.getCpuFrequency();

	response.sendDataStream(stream, MIME_JSON);
}

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/ajax/input", onAjaxInput);
	server.paths.set("/ajax/frequency", onAjaxFrequency);
	server.paths.setDefault(onFile);

	Serial << endl
		   << _F("=== WEB SERVER STARTED ===") << endl
		   << WifiStation.getIP() << endl
		   << _F("==========================") << endl
		   << endl;
}

void startFTP()
{
	if(!fileExist("index.html"))
		fileSetContent("index.html",
					   F("<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>"));

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	startFTP();
	startWebServer();
}

} // namespace

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	for(auto pin : inputs) {
		pinMode(pin, INPUT);
	}

	// Run our method when station was connected to AP
	WifiEvents.onStationGotIP(gotIP);
}
