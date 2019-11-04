#include <SmingCore.h>

#include "configuration.h"

bool serverStarted = false;
HttpServer server;
extern String StrT, StrRH; // Sensors string values

void onIndex(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	vars["T"] = StrT;
	vars["RH"] = StrRH;
	response.sendNamedStream(tmpl);
}

void onConfiguration(HttpRequest& request, HttpResponse& response)
{
	MeteoConfig cfg = loadConfig();
	if(request.method == HTTP_POST) {
		debugf("Update config");
		// Update config
		if(request.getPostParameter("SSID").length() > 0) // Network
		{
			cfg.NetworkSSID = request.getPostParameter("SSID");
			cfg.NetworkPassword = request.getPostParameter("Password");
		}
		if(request.getPostParameter("TZ").length() > 0) // Correction
		{
			cfg.AddT = request.getPostParameter("T").toFloat();
			cfg.AddRH = request.getPostParameter("RH").toFloat();
			cfg.AddTZ = request.getPostParameter("TZ").toFloat();
		}
		if(request.getPostParameter("Trigger").length() > 0) // Trigger
		{
			cfg.Trigger = (TriggerType)request.getPostParameter("Trigger").toInt();
			cfg.RangeMin = request.getPostParameter("RMin").toFloat();
			cfg.RangeMax = request.getPostParameter("RMax").toFloat();
		}
		saveConfig(cfg);
		startWebClock(); // Apply time zone settings
		response.headers[HTTP_HEADER_LOCATION] = "/";
	}

	debugf("Send template");
	TemplateFileStream* tmpl = new TemplateFileStream("config.html");
	auto& vars = tmpl->variables();
	vars["SSID"] = cfg.NetworkSSID;
	vars["T"] = String(cfg.AddT, 2);
	vars["RH"] = String(cfg.AddRH, 2);
	vars["TZ"] = String(cfg.AddTZ, 2);
	vars["Trigger"] = String((int)cfg.Trigger);
	vars["RMin"] = String(cfg.RangeMin, 2);
	vars["RMax"] = String(cfg.RangeMax, 2);
	response.sendNamedStream(tmpl);
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

/// API ///

void onApiDoc(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("api.html");
	auto& vars = tmpl->variables();
	vars["IP"] = (WifiStation.isConnected() ? WifiStation.getIP() : WifiAccessPoint.getIP()).toString();
	response.sendNamedStream(tmpl);
}

void onApiSensors(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();
	json["status"] = (bool)true;
	JsonObject sensors = json.createNestedObject("sensors");
	sensors["temperature"] = StrT.c_str();
	sensors["humidity"] = StrRH.c_str();
	response.sendDataStream(stream, MIME_JSON);
}

void onApiOutput(HttpRequest& request, HttpResponse& response)
{
	int val = request.getQueryParameter("control", "-1").toInt();
	if(val == 0 || val == 1)
		digitalWrite(CONTROL_PIN, val == 1);
	else
		val = -1;

	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();
	json["status"] = val != -1;
	if(val == -1)
		json["error"] = "Wrong control parameter value, please use: ?control=0|1";
	response.sendDataStream(stream, MIME_JSON);
}

void startWebServer()
{
	if(serverStarted)
		return;

	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/api", onApiDoc);
	server.paths.set("/api/sensors", onApiSensors);
	server.paths.set("/api/output", onApiOutput);
	server.paths.set("/config", onConfiguration);
	server.paths.setDefault(onFile);
	serverStarted = true;

	if(WifiStation.isEnabled())
		debugf("STA: %s", WifiStation.getIP().toString().c_str());
	if(WifiAccessPoint.isEnabled())
		debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}

/// FileSystem Initialization ///

HttpClient downloadClient;
int dowfid = 0;
void downloadContentFiles()
{
	debugf("DownloadContentFiles");

	downloadClient.downloadFile("http://simple.anakod.ru/templates/MeteoControl/MeteoControl.html", "index.html");
	downloadClient.downloadFile("http://simple.anakod.ru/templates/MeteoControl/MeteoConfig.html", "config.html");
	downloadClient.downloadFile("http://simple.anakod.ru/templates/MeteoControl/MeteoAPI.html", "api.html");
	downloadClient.downloadFile("http://simple.anakod.ru/templates/bootstrap.css.gz");
	downloadClient.downloadFile("http://simple.anakod.ru/templates/jquery.js.gz",
								(RequestCompletedDelegate)([](HttpConnection& connection, bool success) -> int {
									if(success) {
										startWebServer();
									}
									return 0;
								}));
}
