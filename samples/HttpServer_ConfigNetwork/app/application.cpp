#include <SmingCore.h>
#include <AppSettings.h>
#include <JsonObjectStream.h>
#include <Data/Stream/TemplateFlashMemoryStream.h>

namespace
{
ApplicationSettingsStorage AppSettings;

HttpServer server;
FtpServer ftp;

HashMap<String, BssInfo> networks;

String network;
String password;
SimpleTimer connectionTimer;

String lastModified;

SimpleTimer scanTimer;

DEFINE_FSTR(DEFAULT_IP, "192.168.1.77")
DEFINE_FSTR(DEFAULT_NETMASK, "255.255.255.0")
DEFINE_FSTR(DEFAULT_GATEWAY, "192.168.1.1")

// Instead of using a SPIFFS file, here we demonstrate usage of imported Flash Strings
IMPORT_FSTR_LOCAL(flashSettings, PROJECT_DIR "/web/build/settings.html")

#ifdef ENABLE_SSL
IMPORT_FSTR_LOCAL(serverKey, PROJECT_DIR "/cert/key_1024");
IMPORT_FSTR_LOCAL(serverCert, PROJECT_DIR "/cert/x509_1024.cer");
#endif

void onIndex(HttpRequest& request, HttpResponse& response)
{
	response.sendFile("index.html");
}

int onIpConfig(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	if(request.method == HTTP_POST) {
		Serial << _F("Request coming from IP: ") << connection.getRemoteIp() << endl;
		// If desired you can also limit the access based on remote IP. Example below:
		//		if(IpAddress("192.168.4.23") != connection.getRemoteIp()) {
		//			return 1; // error
		//		}

		AppSettings.dhcp = request.getPostParameter("dhcp") == "1";
		AppSettings.ip = request.getPostParameter("ip");
		AppSettings.netmask = request.getPostParameter("netmask");
		AppSettings.gateway = request.getPostParameter("gateway");
		Serial << _F("Updating IP settings: ") << AppSettings.ip.isNull() << endl;
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
	vars["dhcpon"] = dhcp ? _F("checked='checked'") : "";
	vars["dhcpoff"] = !dhcp ? _F("checked='checked'") : "";

	auto set = !WifiStation.getIP().isNull();
	vars["ip"] = set ? WifiStation.getIP().toString() : DEFAULT_IP;
	vars["netmask"] = set ? WifiStation.getNetworkMask().toString() : DEFAULT_NETMASK;
	vars["gateway"] = set ? WifiStation.getNetworkGateway().toString() : DEFAULT_GATEWAY;

	response.sendNamedStream(tmpl); // will be automatically deleted

	return 0;
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	if(lastModified.length() > 0 && request.headers[HTTP_HEADER_IF_MODIFIED_SINCE] == lastModified) {
		response.code = HTTP_STATUS_NOT_MODIFIED;
		return;
	}

	String file = request.uri.getRelativePath();

	if(file[0] == '.') {
		response.code = HTTP_STATUS_FORBIDDEN;
	} else {
		if(lastModified.length() > 0) {
			response.headers[HTTP_HEADER_LAST_MODIFIED] = lastModified;
		}

		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onAjaxNetworkList(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream(4096);
	JsonObject json = stream->getRoot();

	json["status"] = (bool)true;

	bool connected = WifiStation.isConnected();
	json["connected"] = connected;
	if(connected) {
		// Copy full string to JSON buffer memory
		json["network"] = WifiStation.getSSID();
	}

	JsonArray netlist = json.createNestedArray("available");
	for(auto nw : networks) {
		if(nw->hidden) {
			continue;
		}
		JsonObject item = netlist.createNestedObject();
		item["id"] = nw->getHashId();
		// Copy full string to JSON buffer memory
		item["title"] = nw->ssid;
		item["signal"] = nw->rssi;
		item["encryption"] = nw->getAuthorizationMethodName();
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendDataStream(stream, MIME_JSON);
}

void makeConnection()
{
	debug_i(">> makeConnection()");
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
			connectionTimer.initializeMs<1200>(makeConnection).startOnce();
		} else {
			json["connected"] = WifiStation.isConnected();
			debugf("Network already selected. Current status: %s", WifiStation.getConnectionStatusName().c_str());
		}
	}

	if(!updating && !connectingNow && WifiStation.isConnectionFailed())
		json["error"] = WifiStation.getConnectionStatusName();

	response.setAllowCrossDomainOrigin("*");
	response.sendDataStream(stream, MIME_JSON);
}

void startWebServer()
{
#ifdef ENABLE_SSL
	server.setSslInitHandler([](Ssl::Session& session) {
		debug_i("SSL Init handler: setting server keyCert");
		session.keyCert.assign(serverKey, serverCert);
	});
	server.listen(443, true);
#else
	server.listen(80);
#endif
	server.paths.set("/", onIndex);
	server.paths.set(F("/ipconfig"), onIpConfig);
	server.paths.set(F("/ajax/get-networks"), onAjaxNetworkList);
	server.paths.set(F("/ajax/connect"), onAjaxConnect);
	server.paths.setDefault(onFile);
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

// Will be called when system initialization was completed
void startServers()
{
	startFTP();
	startWebServer();
}

void scanNetworks();

void networkScanCompleted(bool succeeded, BssList& list)
{
	if(succeeded) {
		networks.clear();
		for(auto& nw : list) {
			if(nw.hidden || nw.ssid.length() == 0) {
				continue;
			}
			int i = networks.indexOf(nw.ssid);
			if(i < 0 || nw.rssi > networks.valueAt(i).rssi) {
				networks[nw.ssid] = nw;
			}
		}

		// networks.sort([](const auto& a, const auto& b) { return b.value().rssi < a.value().rssi; });
	}

	scanTimer.initializeMs<30000>(scanNetworks);
	scanTimer.startOnce();
}

void scanNetworks()
{
	if(!WifiStation.startScan(networkScanCompleted)) {
		scanTimer.initializeMs<5000>(scanNetworks);
		scanTimer.startOnce();
	}
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	spiffs_mount(); // Mount file system, in order to work with files

	if(fileExist(".lastModified")) {
		// The last modification
		lastModified = fileGetContent(".lastModified");
		lastModified.trim();
	}

	AppSettings.load();

	WifiStation.enable(true);

	if(AppSettings.exist()) {
		WifiStation.config(AppSettings.ssid, AppSettings.password);
		if(!AppSettings.dhcp && !AppSettings.ip.isNull()) {
			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);
		}
	}

	scanNetworks();

	// Start AP for configuration
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("Sming Configuration", "", AUTH_OPEN);

	// Run WEB server on system ready
	System.onReady(startServers);
}
