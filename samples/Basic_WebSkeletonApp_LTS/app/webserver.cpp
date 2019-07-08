#include <tytherm.h>

bool serverStarted = false;
HttpServer server;

void onIndex(HttpRequest& request, HttpResponse& response)
{
	response.setCache(86400, true); // It's important to use cache for better performance.
	response.sendFile("index.html");
}

void onConfiguration(HttpRequest& request, HttpResponse& response)
{
	if(request.method == HTTP_POST) {
		debugf("Update config");
		// Update config
		if(request.getBody() == nullptr) {
			debugf("NULL bodyBuf");
			return;
		} else {
			StaticJsonBuffer<ConfigJsonBufferSize> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			root.prettyPrintTo(Serial); //Uncomment it for debuging

			if(root["StaSSID"].success()) // Settings
			{
				uint8_t PrevStaEnable = ActiveConfig.StaEnable;

				ActiveConfig.StaSSID = String((const char*)root["StaSSID"]);
				ActiveConfig.StaPassword = String((const char*)root["StaPassword"]);
				ActiveConfig.StaEnable = root["StaEnable"];

				if(PrevStaEnable && ActiveConfig.StaEnable) {
					WifiStation.enable(true);
					WifiAccessPoint.enable(false);
					WifiStation.config(ActiveConfig.StaSSID, ActiveConfig.StaPassword);
				} else if(ActiveConfig.StaEnable) {
					WifiStation.enable(true, true);
					WifiAccessPoint.enable(false, true);
					WifiStation.config(ActiveConfig.StaSSID, ActiveConfig.StaPassword);
				} else {
					WifiStation.enable(false, true);
					WifiAccessPoint.enable(true, true);
					WifiAccessPoint.config("TyTherm", "ENTERYOURPASSWD", AUTH_WPA2_PSK);
				}
			}
		}
		saveConfig(ActiveConfig);
	} else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile("config.html");
	}
}

void onConfigurationJson(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["StaSSID"] = ActiveConfig.StaSSID;
	json["StaPassword"] = ActiveConfig.StaPassword;
	json["StaEnable"] = ActiveConfig.StaEnable;

	response.sendDataStream(stream, MIME_JSON);
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

void onAjaxGetState(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["counter"] = counter;

	response.sendDataStream(stream, MIME_JSON);
}

void startWebServer()
{
	if(serverStarted)
		return;

	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/config", onConfiguration);
	server.paths.set("/config.json", onConfigurationJson);
	server.paths.set("/state", onAjaxGetState);
	server.paths.setDefault(onFile);
	server.setBodyParser("application/json", bodyToStringParser);
	serverStarted = true;

	if(WifiStation.isEnabled())
		debugf("STA: %s", WifiStation.getIP().toString().c_str());
	if(WifiAccessPoint.isEnabled())
		debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}
