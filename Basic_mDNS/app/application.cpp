#include <user_config.h>
#include <SmingCore/SmingCore.h>

/*** mDNS Demo
 * The multicast Domain Name System (mDNS) resolves host names to IP addresses
 * within small networks that do not include a local name server.
 * More info on mDNS can be found at https://en.wikipedia.org/wiki/Multicast_DNS
 * mDNS has two parts 1. Advertise 2. Listen
 * Bellow code (using ESP SDK) just does Advertise
 * Listen implementation is still on going work and will be basically porting
 * http://gkaindl.com/software/arduino-ethernet/bonjour to "Sming"
 *
 * In short this code will advertise other machines about its ipaddress.
 * But you can not convert other mDNS advertiser's host name to ipaddress. (this is work of Listening)
 *
 * How to use mDNS
 * 1. ADD your WIFI_SSID / Password
 * 2. Flash the Complied code to ESP8266
 * 3. According to OS of your PC / phone
 * 		A. Mac OS (iphone/ipad/ mac)
 * 			in Safari browser type "http://test.local/" to open the Ajax sample webpage.
 * 		B. Windows (You need Bonjour Service running. If you do not have it Install it from
 * 		   http://download.cnet.com/Bonjour-for-Windows/3000-18507_4-93550.html
 * 		   After installing in IE or Chrome or other browser type
 * 		   "http://test.local/" to open the Ajax sample webpage.
 * 		C. Linux  (You need to install Avahi mDNS/DNS-SD daemon)
 * 			In your browser type "http://test.local/" to open the Ajax sample webpage.
 * 		D. Android
 * 			You need to install ZeroConf Browser or Bonjour Browser
 * 			In those app you would be able to see IP address of your ESP module
 * 			In android Chrome "http://test.local/" does not translate to IPaddres
 * 			So android Chrome is not supporting mDNS.
 * 			But you can make your own app using Network Service Discovery. Look at bellow link for details
 * 			http://developer.android.com/training/connect-devices-wirelessly/nsd.html
 *
 */

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

HttpServer server;
FTPServer ftp;

int inputs[] = {0, 2}; // Set input GPIO pins here
Vector<String> namesInput;
const int countInputs = sizeof(inputs) /  sizeof(inputs[0]);

//mDNS using ESP8266 SDK functions
void startmDNS() {
    struct mdns_info *info = (struct mdns_info *)os_zalloc(sizeof(struct mdns_info));
    info->host_name = (char *) "test"; // You can replace test with your own host name
    info->ipAddr = WifiStation.getIP();
    info->server_name = (char *) "Sming";
    info->server_port = 80;
    info->txt_data[0] = (char *) "version = now";
    espconn_mdns_init(info);
}

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendTemplate(tmpl); // this template object will be deleted automatically
}

void onFile(HttpRequest &request, HttpResponse &response)
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

void onAjaxInput(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	json["status"] = (bool)true;

	JsonObject& gpio = json.createNestedObject("gpio");
	for (int i = 0; i < countInputs; i++)
		gpio[namesInput[i].c_str()] = digitalRead(inputs[i]);

	response.sendJsonObject(stream);
}

void onAjaxFrequency(HttpRequest &request, HttpResponse &response)
{
	int freq = request.getQueryParameter("value").toInt();
	System.setCpuFrequency((CpuFrequency)freq);

	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	json["status"] = (bool)true;
	json["value"] = (int)System.getCpuFrequency();

	response.sendJsonObject(stream);
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/ajax/input", onAjaxInput);
	server.addPath("/ajax/frequency", onAjaxFrequency);
	server.setDefaultHandler(onFile);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
}

void startFTP()
{
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

	startFTP();
	startWebServer();
	startmDNS();  // Start mDNS "Advertise" of your hostname "test.local" for this example
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	for (int i = 0; i < countInputs; i++)
	{
		namesInput.add(String(inputs[i]));
		pinMode(inputs[i], INPUT);
	}

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk);
}
