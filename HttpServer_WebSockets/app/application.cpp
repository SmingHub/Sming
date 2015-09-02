#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Network/TelnetServer.h>


// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

HttpServer server;
int totalActiveSockets = 0;
FTPServer ftp;
Timer msgTimer;
TelnetServer telnet;

void commandOptions(String commandLine  ,CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ' , commandToken);
	bool errorCommand = false;
	bool printUsage = false;

	switch (numToken)
	{
		case 2 :
			if (commandToken[1] == "help")
			{
				printUsage = true;
			}
			if (commandToken[1] == "verbose")
			{
				commandHandler.setVerboseMode(VERBOSE);
				commandOutput->printf("Verbose mode selected\r\n");
				break;
			}
			if (commandToken[1] == "silent")
			{
				commandHandler.setVerboseMode(SILENT);
				commandOutput->printf("Silent mode selected\r\n");
				break;
			}
			errorCommand = true;
			break;
		case 3 :
			if (commandToken[1] != "prompt")
			{
				errorCommand = true;
				break;
			}
			commandHandler.setCommandPrompt(commandToken[2]);
			commandOutput->printf("Prompt set to : %s\r\n",commandToken[2].c_str());
			break;
		default :
			errorCommand = true;
	}
	if (errorCommand)
	{
		commandOutput->printf("Unknown command : %s\r\n",commandLine.c_str());
	}
	if (printUsage)
	{
		commandOutput->printf("command usage : \r\n\r\n");
		commandOutput->printf("command verbose : Set verbose mode\r\n");
		commandOutput->printf("command silent : Set silent mode\r\n");
		commandOutput->printf("command prompt 'new prompt' : Set prompt to use\r\n");
	}
}


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

void msgTimerDelegate()
{
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++)
		clients[i].sendString("New message for all clients : " + String(msgCount++));
}

void wsConnected(WebSocket& socket)
{
	totalActiveSockets++;
	Serial.printf("Socket connected\r\n");
	// Notify everybody about new connection
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++)
		clients[i].sendString("New friend arrived! Total: " + String(totalActiveSockets));
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
	totalActiveSockets--;

	// Notify everybody about lost connection
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++)
		clients[i].sendString("We lost our friend :( Total: " + String(totalActiveSockets));
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

	server.enableWebSocketsCommand(true,"command");

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account

	Serial.println("\r\n=== FTP SERVER STARTED ===");
	Serial.println("==============================\r\n");

	telnet.listen(23);

	Serial.println("\r\n=== TelnetServer SERVER STARTED ===");
	Serial.println("==============================\r\n");



//	msgTimer.initializeMs(2000,msgTimerDelegate).start();
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

	startWebServer();
}

void init()
{
	Serial.begin(74880); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial
	Serial.commandProcessing(true);
	commandHandler.registerSystemCommands();
	commandHandler.registerCommand(CommandDelegate("command","Usage verbose/silent/prompt for command options\r\n","testGroup", commandOptions));


	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk);
}
