#include <tytherm.h>
#include <JsonObjectStream.h>
#include <FlashString/Map.hpp>
#include <FlashString/Stream.hpp>
#include "DelayStream.h"

namespace
{
bool serverStarted = false;
HttpServer server;

#if DISABLE_SPIFFS

// If a filesystem image hasn't been provided, serve the files using a FlashString map
#define FILE_LIST(XX)                                                                                                  \
	XX(bootstrap, "bootstrap.min.css.gz")                                                                              \
	XX(config_html, "config.html")                                                                                     \
	XX(config_js, "config.js")                                                                                         \
	XX(index_html, "index.html")                                                                                       \
	XX(index_js, "index.js")                                                                                           \
	XX(jquery, "jquery-2.1.4.min.js.gz")

// Define the names for each file
#define XX(name, file) DEFINE_FSTR_LOCAL(KEY_##name, file)
FILE_LIST(XX)
#undef XX

// Import content for each file
#define XX(name, file) IMPORT_FSTR_LOCAL(CONTENT_##name, PROJECT_DIR "/files/" file);
FILE_LIST(XX)
#undef XX

// Define the table structure linking key => content
#define XX(name, file) {&KEY_##name, &CONTENT_##name},
DEFINE_FSTR_MAP_LOCAL(fileMap, FlashString, FlashString, FILE_LIST(XX));
#undef XX

void sendFile(const String& fileName, HttpServerConnection& connection)
{
	auto response = connection.getResponse();

	String compressed = fileName + ".gz";
	auto v = fileMap[compressed];
	if(v) {
		response->headers[HTTP_HEADER_CONTENT_ENCODING] = _F("gzip");
	} else {
		v = fileMap[fileName];
		if(!v) {
			debug_w("File '%s' not found", fileName.c_str());
			response->code = HTTP_STATUS_NOT_FOUND;
			return;
		}
	}

	debug_i("found %s in fileMap", String(v.key()).c_str());
	auto stream = new FSTR::Stream(v.content());
	response->sendDataStream(stream, ContentType::fromFullFileName(fileName));

	// Use client caching for better performance.
	//	response->setCache(86400, true);
}

#else

void sendFile(const String& fileName, HttpServerConnection& connection)
{
	// Send file directly
	//	response.sendFile(fileName);

	// Use custom stream to defer response
	debug_i("File '%s' requested", fileName.c_str());
	auto stream = new DelayStream(fileName, connection);
	connection.getResponse()->sendDataStream(stream);

	// Use client caching for better performance.
	//	connection.getResponse()->setCache(86400, true);
}

#endif

int onIndex(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	sendFile("index.html", connection);

	return 0;
}

int onConfiguration(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	if(request.method == HTTP_GET) {
		sendFile("config.html", connection);
		return 0;
	}

	if(request.method != HTTP_POST) {
		response.code = HTTP_STATUS_BAD_REQUEST;
		return 0;
	}

	debugf("Update config");
	// Update config
	if(request.getBody() == nullptr) {
		debugf("NULL bodyBuf");
		return 0;
	}

	StaticJsonDocument<ConfigJsonBufferSize> root;

	if(!Json::deserialize(root, request.getBodyStream())) {
		debug_w("Invalid JSON to un-serialize");
		return 0;
	}

	Json::serialize(root, Serial, Json::Pretty); // For debugging

	if(root.containsKey("StaSSID")) // Settings
	{
		uint8_t prevStaEnable = activeConfig.StaEnable;

		activeConfig.StaSSID = String((const char*)root["StaSSID"]);
		activeConfig.StaPassword = String((const char*)root["StaPassword"]);
		activeConfig.StaEnable = root["StaEnable"];

		if(prevStaEnable && activeConfig.StaEnable) {
			WifiStation.enable(true);
			WifiAccessPoint.enable(false);
			WifiStation.config(activeConfig.StaSSID, activeConfig.StaPassword);
		} else if(activeConfig.StaEnable) {
			WifiStation.enable(true, true);
			WifiAccessPoint.enable(false, true);
			WifiStation.config(activeConfig.StaSSID, activeConfig.StaPassword);
		} else {
			WifiStation.enable(false, true);
			WifiAccessPoint.enable(true, true);
			WifiAccessPoint.config("TyTherm", "ENTERYOURPASSWD", AUTH_WPA2_PSK);
		}
	}

	saveConfig(activeConfig);
	return 0;
}

void onConfigurationJson(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();

	json["StaSSID"] = activeConfig.StaSSID;
	json["StaPassword"] = activeConfig.StaPassword;
	json["StaEnable"] = activeConfig.StaEnable;

	response.sendDataStream(stream, MIME_JSON);
}

int onFile(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();

	if(file[0] == '.') {
		response.code = HTTP_STATUS_FORBIDDEN;
	} else {
		//		response.setCache(86400, true); // It's important to use cache for better performance.
		sendFile(file, connection);
	}

	return 0;
}

void onAjaxGetState(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();

	json["counter"] = counter;

	response.sendDataStream(stream, MIME_JSON);
}

} // namespace

void startWebServer()
{
	if(serverStarted) {
		return;
	}

	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/config", onConfiguration);
	server.paths.set("/config.json", onConfigurationJson);
	server.paths.set("/state", onAjaxGetState);
	server.paths.setDefault(onFile);
	server.setBodyParser(MIME_JSON, bodyToStringParser);
	serverStarted = true;

	if(WifiStation.isEnabled()) {
		debugf("STA: %s", WifiStation.getIP().toString().c_str());
	}
	if(WifiAccessPoint.isEnabled()) {
		debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
	}
}
