#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Network/TelnetServer.h>
#include <SmingCore/Debug.h>
#include <ExampleCommand.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

HttpServer server;
FTPServer ftp;
TelnetServer telnet;

Timer msgTimer;

ExampleCommand exampleCommand;


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

int msgCount = 0;

void wsConnected(WebSocket& socket)
{
	Serial.printf("Socket connected\r\n");
}

void wsMessageReceived(WebSocket& socket, const String& message)
{
	Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
	String response = "Echo: " + message;
	socket.sendString(response);
}

void wsBinaryReceived(WebSocket& socket, uint8_t* data, size_t size)
{
	Serial.printf("Websocket binary data recieved, size: %d\r\n", size);
}

void wsDisconnected(WebSocket& socket)
{
	Serial.printf("Socket disconnected");
}

void processApplicationCommands(String commandLine, CommandOutput* commandOutput)
{
	commandOutput->printf("This command is handle by the application\r\n");
}

void StartServers()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.setDefaultHandler(onFile);

	// Web Sockets configuration
	server.enableWebSockets(true);
	server.commandProcessing(true,"command");

	server.setWebSocketConnectionHandler(wsConnected);
	server.setWebSocketMessageHandler(wsMessageReceived);
	server.setWebSocketBinaryHandler(wsBinaryReceived);
	server.setWebSocketDisconnectionHandler(wsDisconnected);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account

	Serial.println("\r\n=== FTP SERVER STARTED ===");
	Serial.println("==============================\r\n");

	telnet.listen(23);
	telnet.enableDebug(true);

	Serial.println("\r\n=== TelnetServer SERVER STARTED ===");
	Serial.println("==============================\r\n");
}

void startExampleApplicationCommand()
{
	exampleCommand.initCommand();
	commandHandler.registerCommand(CommandDelegate("example","Example Command from Class","Application",processApplicationCommands));
}


// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");
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
	WifiStation.waitConnection(connectOk);
}
