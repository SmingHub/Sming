#include <SmingCore.h>
#include <AppSettings.h>
#include "Data/Stream/TemplateFlashMemoryStream.h"

HttpServer server;
FtpServer ftp;

BssList networks;
String network, password;
Timer connectionTimer;

String lastModified;

// Instead of using a SPIFFS file, here we demonstrate usage of imported Flash Strings
IMPORT_FSTR(flashSettings, "web/build/settings.html")

void onIndex(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	response.sendTemplate(tmpl); // will be automatically deleted
}

int onIpConfig(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	if(request.method == HTTP_POST) {
		debugf("Request coming from IP: %s", connection.getRemoteIp().toString().c_str());
		// If desired you can also limit the access based on remote IP. Example below:
		//		if(!(IPAddress("192.168.4.23") == connection.getRemoteIp())) {
		//			return 1; // error
		//		}

		AppSettings.dhcp = request.getPostParameter("dhcp") == "1";
		AppSettings.ip = request.getPostParameter("ip");
		AppSettings.netmask = request.getPostParameter("netmask");
		AppSettings.gateway = request.getPostParameter("gateway");
		debugf("Updating IP settings: %d", AppSettings.ip.isNull());
		AppSettings.save();
	}

	/*
	 * We could use a regular SPIFFS file for this, but instead we demonstrate using a Flash String.
	 *
	 * 	TemplateFileStream* tmpl = new TemplateFileStream("settings.html");
	 */
	auto tmpl = new TemplateFlashMemoryStream(flashSettings);
	auto& vars = tmpl->variables();

	bool dhcp = WifiStation.isEnabledDHCP();
	vars["dhcpon"] = dhcp ? "checked='checked'" : "";
	vars["dhcpoff"] = !dhcp ? "checked='checked'" : "";

	if(!WifiStation.getIP().isNull()) {
		vars["ip"] = WifiStation.getIP().toString();
		vars["netmask"] = WifiStation.getNetworkMask().toString();
		vars["gateway"] = WifiStation.getNetworkGateway().toString();
	} else {
		vars["ip"] = "192.168.1.77";
		vars["netmask"] = "255.255.255.0";
		vars["gateway"] = "192.168.1.1";
	}

	response.sendTemplate(tmpl); // will be automatically deleted

	return 0;
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	if(lastModified.length() > 0 && request.headers[HTTP_HEADER_IF_MODIFIED_SINCE].equals(lastModified)) {
		response.code = HTTP_STATUS_NOT_MODIFIED;
		return;
	}

	String file = request.uri.getRelativePath();

	if(file[0] == '.')
		response.code = HTTP_STATUS_FORBIDDEN;
	else {
		if(lastModified.length() > 0) {
			response.headers[HTTP_HEADER_LAST_MODIFIED] = lastModified;
		}

		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onAjaxNetworkList(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();

	json["status"] = (bool)true;

	bool connected = WifiStation.isConnected();
	json["connected"] = connected;
	if(connected) {
		// Copy full string to JSON buffer memory
		json["network"] = WifiStation.getSSID();
	}

	JsonArray netlist = json.createNestedArray("available");
	for(unsigned i = 0; i < networks.count(); i++) {
		if(networks[i].hidden)
			continue;
		JsonObject item = netlist.createNestedObject();
		item["id"] = (int)networks[i].getHashId();
		// Copy full string to JSON buffer memory
		item["title"] = networks[i].ssid;
		item["signal"] = networks[i].rssi;
		item["encryption"] = networks[i].getAuthorizationMethodName();
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendDataStream(stream, MIME_JSON);
}

void makeConnection()
{
	WifiStation.enable(true);
	WifiStation.config(network, password);

	AppSettings.ssid = network;
	AppSettings.password = password;
	AppSettings.save();

	network = ""; // task completed
}

void onAjaxConnect(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();

	String curNet = request.getPostParameter("network");
	String curPass = request.getPostParameter("password");

	bool updating = curNet.length() > 0 && (WifiStation.getSSID() != curNet || WifiStation.getPassword() != curPass);
	bool connectingNow = WifiStation.getConnectionStatus() == eSCS_Connecting || network.length() > 0;

	if(updating && connectingNow) {
		debugf("wrong action: %s %s, (updating: %d, connectingNow: %d)", network.c_str(), password.c_str(), updating,
			   connectingNow);
		json["status"] = (bool)false;
		json["connected"] = (bool)false;
	} else {
		json["status"] = (bool)true;
		if(updating) {
			network = curNet;
			password = curPass;
			debugf("CONNECT TO: %s %s", network.c_str(), password.c_str());
			json["connected"] = false;
			connectionTimer.initializeMs(1200, makeConnection).startOnce();
		} else {
			json["connected"] = WifiStation.isConnected();
			debugf("Network already selected. Current status: %s", WifiStation.getConnectionStatusName());
		}
	}

	if(!updating && !connectingNow && WifiStation.isConnectionFailed())
		json["error"] = WifiStation.getConnectionStatusName();

	response.setAllowCrossDomainOrigin("*");
	response.sendDataStream(stream, MIME_JSON);
}

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/ipconfig", onIpConfig);
	server.paths.set("/ajax/get-networks", onAjaxNetworkList);
	server.paths.set("/ajax/connect", onAjaxConnect);
	server.paths.setDefault(onFile);
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

// Will be called when system initialization was completed
void startServers()
{
	startFTP();
	startWebServer();
}

void networkScanCompleted(bool succeeded, BssList list)
{
	if(succeeded) {
		for(unsigned i = 0; i < list.count(); i++)
			if(!list[i].hidden && list[i].ssid.length() > 0)
				networks.add(list[i]);
	}
	networks.sort([](const BssInfo& a, const BssInfo& b) { return b.rssi - a.rssi; });
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	if(fileExist(".lastModified")) {
		// The last modification
		lastModified = fileGetContent(".lastModified");
		lastModified.trim();
	}

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial
	AppSettings.load();

	WifiStation.enable(true);

	if(AppSettings.exist()) {
		WifiStation.config(AppSettings.ssid, AppSettings.password);
		if(!AppSettings.dhcp && !AppSettings.ip.isNull())
			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);
	}

	WifiStation.startScan(networkScanCompleted);

	// Start AP for configuration
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("Sming Configuration", "", AUTH_OPEN);

	// Run WEB server on system ready
	System.onReady(startServers);
}
