#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Wiring/SplitString.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

Timer procTimer;

HttpServer server;
int totalActiveSockets = 0;
long nextPos[4];
long curPos[4];
uint8_t step[4];
uint8_t dir[4];

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

void wsConnected(WebSocket& socket)
{
	totalActiveSockets++;

	// Notify everybody about new connection
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++)
	{
		clients[i].sendString(
				"New friend arrived! Total: " + String(totalActiveSockets));
	}
}

void wsMessageReceived(WebSocket& socket, const String& message)
{
	Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
	char buf[2];
	sprintf(buf, "OK");
	socket.sendString(buf);

	String commandLine;
	commandLine = message.c_str();

	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);
	for (int i = 0; i < numToken; i++)
	{
		Serial.printf("Command: \r\n %s \r\n", commandToken[i].c_str());
		String motor = commandToken[i].substring(0, 1);
		String posStr = commandToken[i].substring(1, commandToken[i].length());
		int8_t index = -1;
		if (motor == "X")
			index = 0;
		else if (motor == "Y")
			index = 1;
		else if (motor == "Z")
			index = 2;
		else if (motor == "E")
			index = 3;
		if (index > -1)
		{
			nextPos[index] = atol(posStr.c_str());
			Serial.printf("Set nextpos[%d] to %d\r\n", index, nextPos[index]);
		}
	}

}

void wsBinaryReceived(WebSocket& socket, uint8_t* data, size_t size)
{
	Serial.printf("Websocket binary data receieved, size: %d\r\n", size);
}

void wsDisconnected(WebSocket& socket)
{
	totalActiveSockets--;

// Notify everybody about lost connection
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++)
		clients[i].sendString(
				"We lost our friend :( Total: " + String(totalActiveSockets));
}

void blink1()
{
	system_soft_wdt_feed();
	for (int i = 0; i < 4; i++)
	{
		if (curPos[i] != nextPos[i])
		{
			int8_t sign = -1;
			if (nextPos[i] > curPos[i])
				sign = 1;
			if (sign > 0)
				digitalWrite(dir[i], 0);
			else
				digitalWrite(dir[i], 1);
			delayMicroseconds(5);

			digitalWrite(step[i], true);
			delayMicroseconds(10);
			digitalWrite(step[i], false);
			curPos[i] = curPos[i] + sign;
		}
	}
	procTimer.initializeUs(100, blink1).startOnce();
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.setDefaultHandler(onFile);

// Web Sockets configuration
	server.enableWebSockets(true);
	server.setWebSocketConnectionHandler(wsConnected);
	server.setWebSocketMessageHandler(wsMessageReceived);
	server.setWebSocketBinaryHandler(wsBinaryReceived);
	server.setWebSocketDisconnectionHandler(wsDisconnected);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");

	procTimer.initializeUs(100, blink1).startOnce();
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");
	Serial.println("IP: ");
	Serial.println(WifiStation.getIP().toString());
	Serial.println("Starting web server...");
	startWebServer();

}



void couldntConnect()
{
	Serial.println("Couldn't connect");
}

void init()
{
	Serial.systemDebugOutput(true);
	System.setCpuFrequency(eCF_160MHz);
	system_soft_wdt_stop();

	Serial.println("Init running...");

	step[0] = 15;
	dir[0] = 2;

	step[1] = 0;
	dir[1] = 5;

	step[2] = 4;
	dir[2] = 13;

	step[3] = 12;
	dir[3] = 14;

	for (int i = 0; i < 4; i++)
	{
		pinMode(step[i], OUTPUT);
		pinMode(dir[i], OUTPUT);
		curPos[i] = 0;
		nextPos[i] = 0;
	}

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	WifiStation.waitConnection(connectOk);

}
