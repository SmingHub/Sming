#include <SmingCore.h>
#include <Network/Http/Websocket/WebsocketResource.h>
#include "CUserData.h"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

HttpServer server;
unsigned totalActiveSockets = 0;

CUserData userGeorge("George", "I like SMING");

void onIndex(HttpRequest& request, HttpResponse& response)
{
	auto tmpl = new TemplateFileStream(F("index.html"));
	auto& vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendNamedStream(tmpl); // this template object will be deleted automatically
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();

	if(file[0] == '.') {
		response.code = HTTP_STATUS_FORBIDDEN;
	} else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void wsConnected(WebsocketConnection& socket)
{
	totalActiveSockets++;

	//Use a global instance and add this new connection. Normally
	userGeorge.addSession(socket);
	// Notify everybody about new connection

	String message = F("New friend arrived! Total: ") + String(totalActiveSockets);
	socket.broadcast(message);
}

void wsMessageReceived(WebsocketConnection& socket, const String& message)
{
	Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());

	if(message == _F("shutdown")) {
		String message(F("The server is shutting down..."));
		socket.broadcast(message);

		// Don't shutdown immediately, wait a bit to allow messages to propagate
		auto timer = new SimpleTimer;
		timer->setCallback(
			[](void* timer) {
				delete static_cast<SimpleTimer*>(timer);
				server.shutdown();
			},
			timer);
		timer->startMs(1000);
		return;
	}

	String response = F("Echo: ") + message;
	socket.sendString(response);

	//Normally you would use dynamic cast but just be careful not to convert to wrong object type!
	auto user = reinterpret_cast<CUserData*>(socket.getUserData());
	if(user != nullptr) {
		user->printMessage(socket, message);
	}
}

void wsBinaryReceived(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	Serial.printf(_F("Websocket binary data recieved, size: %d\r\n"), size);
}

void wsDisconnected(WebsocketConnection& socket)
{
	totalActiveSockets--;

	//Normally you would use dynamic cast but just be careful not to convert to wrong object type!
	auto user = reinterpret_cast<CUserData*>(socket.getUserData());
	if(user != nullptr) {
		user->removeSession(socket);
	}

	// Notify everybody about lost connection
	String message = F("We lost our friend :( Total: ") + String(totalActiveSockets);
	socket.broadcast(message);
}

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.setDefault(onFile);

	// Web Sockets configuration
	auto wsResource = new WebsocketResource();
	wsResource->setConnectionHandler(wsConnected);
	wsResource->setMessageHandler(wsMessageReceived);
	wsResource->setBinaryHandler(wsBinaryReceived);
	wsResource->setDisconnectionHandler(wsDisconnected);

	server.paths.set("/ws", wsResource);

	Serial.println(F("\r\n=== WEB SERVER STARTED ==="));
	Serial.println(WifiStation.getIP());
	Serial.println(F("==============================\r\n"));
}

// Will be called when WiFi station becomes fully operational
void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
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

	// Run our method when station was connected to AP
	WifiEvents.onStationGotIP(gotIP);
}
