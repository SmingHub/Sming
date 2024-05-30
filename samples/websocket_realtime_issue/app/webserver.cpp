#include <SmingCore.h>
#include <Network/Http/Websocket/WebsocketResource.h>
#include <JsonObjectStream.h>
#include "webserver.h"

HttpServer server;
Timer websocket_update_timer;
int i{};

void onIndex(HttpRequest& request, HttpResponse& response)
{
	auto tmpl = new TemplateFileStream(F("index.html"));
	response.sendNamedStream(tmpl); // this template object will be deleted automatically
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();
	response.setCache(86400, true); // problematic when sending modifiable json
	response.sendFile(file);
}

void send(WebsocketConnection& socket, int i)
{
	StaticJsonDocument<128> doc;
	auto json = doc.to<JsonObject>();
	auto obj = json.createNestedObject("data");
	obj["i"] = i;
	socket.send(Json::serialize(json));
	Serial << "WS sent " << i << endl;
}

void wsConnected(WebsocketConnection& socket)
{
	if(!websocket_update_timer.isStarted()) {
		websocket_update_timer.initializeMs<250>([&]() { send(socket, i++); }).start();
	}
	//String message = F("New friend arrived! ");
	//socket.broadcast(message);
}

void wsMessageReceived(WebsocketConnection& socket, const String& message)
{
	Serial.println(_F("WebSocket message received:"));
	Serial.println(message);

	if(message == _F("shutdown")) {
		String message(F("The server is shutting down..."));
		socket.broadcast(message);
		//shutdownServer();
		return;
	}

	String response = F("Echo: ") + message;
	socket.sendString(response);
}

void wsBinaryReceived(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	Serial << _F("Websocket binary data received, size: ") << size << endl;
}

void wsDisconnected(WebsocketConnection& socket)
{
	// Notify everybody about lost connection
	String message = F("We lost our friend");
	socket.broadcast(message);
	websocket_update_timer.stop();
}

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);

	server.paths.setDefault(onFile);
	server.setBodyParser(MIME_JSON, bodyToStringParser);

	auto wsResource = new WebsocketResource();
	wsResource->setConnectionHandler(wsConnected);
	wsResource->setMessageHandler(wsMessageReceived);
	wsResource->setBinaryHandler(wsBinaryReceived);
	wsResource->setDisconnectionHandler(wsDisconnected);

	server.paths.set("/ws", wsResource);

	Serial.println(_F("\r\n"
					  "=== WEB SERVER STARTED ==="));
	Serial.println(_F("==========================\r\n"));
}
