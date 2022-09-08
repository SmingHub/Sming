#include <SmingCore.h>
#include <Network/TelnetServer.h>
#include <Network/Http/Websocket/WebsocketResource.h>
#include <Debug.h>
#include <ExampleCommand.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

HttpServer server;
FtpServer ftp;
TelnetServer telnet;

Timer msgTimer;

ExampleCommand exampleCommand;

void onIndex(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendNamedStream(tmpl); // this template object will be deleted automatically
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

int msgCount = 0;

void wsConnected(WebsocketConnection& socket)
{
	Serial.println(_F("Socket connected"));
}

void wsMessageReceived(WebsocketConnection& socket, const String& message)
{
	Serial.println(_F("WebsocketConnection message received:"));
	Serial.println(message);
	String response = "Echo: " + message;
	socket.sendString(response);
}

void wsBinaryReceived(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	Serial << _F("Websocket binary data received, size: ") << size << endl;
}

void wsDisconnected(WebsocketConnection& socket)
{
	Serial.println(_F("Socket disconnected"));
}

void processApplicationCommands(String commandLine, CommandOutput* commandOutput)
{
	commandOutput->println(_F("This command is handle by the application"));
}

void StartServers()
{
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.setDefault(onFile);

	// Web Sockets configuration
	WebsocketResource* wsResource = new WebsocketResource();
	wsResource->setConnectionHandler(wsConnected);
	wsResource->setMessageHandler(wsMessageReceived);
	wsResource->setBinaryHandler(wsBinaryReceived);
	wsResource->setDisconnectionHandler(wsDisconnected);

	server.paths.set("/ws", wsResource);

	Serial.println(_F("\r\n=== WEB SERVER STARTED ==="));
	Serial.println(WifiStation.getIP());
	Serial.println(_F("==============================\r\n"));

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account

	Serial.println(_F("\r\n=== FTP SERVER STARTED ==="));
	Serial.println(_F("==============================\r\n"));

	telnet.listen(23);
	telnet.enableDebug(true);

	Serial.println(_F("\r\n=== TelnetServer SERVER STARTED ==="));
	Serial.println(_F("==============================\r\n"));
}

void startExampleApplicationCommand()
{
	exampleCommand.initCommand();
	commandHandler.registerCommand(
		CommandDelegate(F("example"), F("Example Command from Class"), F("Application"), processApplicationCommands));
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	StartServers();

	startExampleApplicationCommand();
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	commandHandler.registerSystemCommands();
	Debug.setDebug(Serial);

	Serial.systemDebugOutput(true); // Enable debug output to serial
	Serial.commandProcessing(true);

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiEvents.onStationGotIP(gotIP);
}
