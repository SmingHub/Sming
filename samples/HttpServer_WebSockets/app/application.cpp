#include <SmingCore.h>
#include <Network/Http/Websocket/WebsocketResource.h>
#include "CUserData.h"

#ifdef ENABLE_CMD_HANDLER
#include <CommandProcessing/Handler.h>
CommandProcessing::Handler commandHandler;
#endif

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
HttpServer server;
unsigned totalActiveSockets;

CUserData userGeorge;

void onIndex(HttpRequest& request, HttpResponse& response)
{
	auto tmpl = new TemplateFileStream(F("index.html"));
	//auto& vars = tmpl->variables();
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

void shutdownServer()
{
	// Don't shutdown immediately, wait a bit to allow messages to propagate
	auto timer = new AutoDeleteTimer;
	timer->initializeMs<1000>([&]() { server.shutdown(); });
	timer->startOnce();
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
	Serial.println(_F("WebSocket message received:"));
	Serial.println(message);

	if(message == _F("shutdown")) {
		String reply(F("The server is shutting down..."));
		socket.broadcast(reply);
		shutdownServer();
		return;
	}

	String response = F("Echo: ") + message;
	socket.sendString(response);

	// Normally you would use dynamic cast but just be careful not to convert to wrong object type!
	auto user = static_cast<CUserData*>(socket.getUserData());
	if(user != nullptr) {
		user->printMessage(socket, message);
	}
}

#ifdef ENABLE_CMD_HANDLER
void wsCommandReceived(WebsocketConnection& socket, const String& message)
{
	debug_i("%s(%s)", __FUNCTION__, message.c_str());
	String response = commandHandler.processNow(message.c_str(), message.length());
	socket.sendString(response);

	// Normally you would use dynamic cast but just be careful not to convert to wrong object type!
	auto user = static_cast<CUserData*>(socket.getUserData());
	if(user != nullptr) {
		user->printMessage(socket, message);
	}
}

void processShutdownCommand(String commandLine, ReadWriteStream& commandOutput)
{
	shutdownServer();
}
#endif

void wsBinaryReceived(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	Serial << _F("Websocket binary data received, size: ") << size << endl;
}

void wsDisconnected(WebsocketConnection& socket)
{
	totalActiveSockets--;

	// Normally you would use dynamic cast but just be careful not to convert to wrong object type!
	auto user = static_cast<CUserData*>(socket.getUserData());
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
#ifdef ENABLE_CMD_HANDLER
	wsResource->setMessageHandler(wsCommandReceived);
#endif

	wsResource->setBinaryHandler(wsBinaryReceived);
	wsResource->setDisconnectionHandler(wsDisconnected);

	server.paths.set("/ws", wsResource);

	Serial.println(_F("\r\n"
					  "=== WEB SERVER STARTED ==="));
	Serial.println(WifiStation.getIP());
	Serial.println(_F("==========================\r\n"));
}

// Will be called when WiFi station becomes fully operational
void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	startWebServer();
}

} // namespace

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

#ifdef ENABLE_CMD_HANDLER
	commandHandler.registerSystemCommands();
	commandHandler.registerCommand(
		{CMDP_STRINGS("shutdown", "Shutdown Server Command", "Application"), processShutdownCommand});
#endif

	userGeorge = CUserData{F("George"), F("I like SMING")};

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(true);
	WifiStation.config(F(WIFI_SSID), F(WIFI_PWD));
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiEvents.onStationGotIP(gotIP);
}
