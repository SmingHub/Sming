#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Network/Http/Websocket/WebsocketResource.h>
#include "CUserData.h"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

HttpServer server;
int totalActiveSockets = 0;

CUserData userGeorge("George", "I like SMING");

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

void wsConnected(WebSocketConnection& socket)
{
	totalActiveSockets++;

	//Use a global instance and add this new connection. Normally
	userGeorge.addSession(socket);
	// Notify everybody about new connection

	String message = "New friend arrived! Total: " + String(totalActiveSockets);
	socket.broadcast(message.c_str(), message.length());
}

void wsMessageReceived(WebSocketConnection& socket, const String& message)
{
	Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
	String response = "Echo: " + message;
	socket.sendString(response);

	//Normally you would use dynamic cast but just be careful not to convert to wrong object type!
    CUserData *user = (CUserData*) socket.getUserData();
    if(user)
    {
    	user->printMessage(socket, message);
    }
}

void wsBinaryReceived(WebSocketConnection& socket, uint8_t* data, size_t size)
{
	Serial.printf("Websocket binary data recieved, size: %d\r\n", size);
}

void wsDisconnected(WebSocketConnection& socket)
{
	totalActiveSockets--;

	//Normally you would use dynamic cast but just be careful not to convert to wrong object type!
    CUserData *user = (CUserData*) socket.getUserData();
    if(user)
    {
    	user->removeSession(socket);
    }

	// Notify everybody about lost connection
    String message = "We lost our friend :( Total: " + String(totalActiveSockets);
    socket.broadcast(message.c_str(), message.length());
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.setDefaultHandler(onFile);

	// Web Sockets configuration
	WebsocketResource* wsResource=new WebsocketResource();
	wsResource->setConnectionHandler(wsConnected);
	wsResource->setMessageHandler(wsMessageReceived);
	wsResource->setBinaryHandler(wsBinaryReceived);
	wsResource->setDisconnectionHandler(wsDisconnected);

	server.addPath("/ws", wsResource);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
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
