#include <SmingCore.h>
#include <JsonObjectStream.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

HttpServer server;
FtpServer ftp;

int inputs[] = {0, 2}; // Set input GPIO pins here
Vector<String> namesInput;
const int countInputs = sizeof(inputs) / sizeof(inputs[0]);

void onIndex(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendTemplate(tmpl); // this template object will be deleted automatically
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

	String stringKey = "StringKey";
	String stringValue = "StringValue";

	json[stringKey] = stringValue;

	for(int i = 0; i < 11; i++) {
		char buff[3];
		itoa(i, buff, 10);
		String desiredString = "sensor_";
		desiredString += buff;
		json[desiredString] = desiredString;
	}

	JsonObject gpio = json.createNestedObject("gpio");
	for(int i = 0; i < countInputs; i++)
		gpio[namesInput[i].c_str()] = digitalRead(inputs[i]);

	response.sendDataStream(stream, MIME_JSON);
}

void onAjaxFrequency(HttpRequest& request, HttpResponse& response)
{
	int freq = request.getQueryParameter("value").toInt();
	System.setCpuFrequency((CpuFrequency)freq);

	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();
	json["status"] = (bool)true;
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

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
}

void startFTP()
{
	if(!fileExist("index.html"))
		fileSetContent("index.html",
					   "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	startFTP();
	startWebServer();
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	for(int i = 0; i < countInputs; i++) {
		namesInput.add(String(inputs[i]));
		pinMode(inputs[i], INPUT);
	}

	// Run our method when station was connected to AP
	WifiEvents.onStationGotIP(gotIP);
}
